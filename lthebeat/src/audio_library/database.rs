use crate::audio_library::album::Album;
use crate::audio_library::artist::Artist;
use crate::audio_library::database_query::DatabaseQuery;
use crate::audio_library::track::Track;
use crate::audio_processing::audio_metadata;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use anyhow::anyhow;
use async_walkdir::{Filtering, WalkDir};
use cntp_i18n::{tr, trn};
use contemporary::application::Details;
use contemporary::jobs::job::JobStatus;
use contemporary::jobs::job_manager::{JobManager, Jobling};
use contemporary::jobs::standard_job::StandardJob;
use directories::UserDirs;
use gpui::{App, AppContext, AsyncApp, BorrowAppContext, Global, hash};
use sha2::{Digest, Sha256};
use smol::fs::File;
use smol::io::{AsyncReadExt, BufReader};
use smol::stream::StreamExt;
use sqlx::sqlite::{
    SqliteArguments, SqliteConnectOptions, SqliteJournalMode, SqlitePoolOptions, SqliteRow,
    SqliteSynchronous,
};
use sqlx::{Acquire, Arguments, Error};
use sqlx::{Executor, Row, SqlitePool};
use std::cell::RefCell;
use std::fs::{metadata, remove_dir_all};
use std::path::{Path, PathBuf};
use std::ptr::read;
use std::rc::Rc;
use std::time::{Duration, SystemTime};
use tracing::error;
use url::Url;

pub struct Database {
    pool: Option<SqlitePool>,
    pub is_library_set_up: bool,
}

impl Database {
    pub async fn new(cx: &mut App) -> anyhow::Result<Self> {
        let library_dir = Self::library_dir(cx);
        let pool = Self::open_library_connection(library_dir).await.ok();
        let mut db = Self {
            pool,
            is_library_set_up: false,
        };
        db.update_library_is_set_up(cx).await;
        Ok(db)
    }

    fn library_dir(cx: &mut App) -> PathBuf {
        let details = cx.global::<Details>();

        let directories = details.standard_dirs().unwrap();
        let data_dir = directories.data_dir();

        data_dir.join("library")
    }

    async fn open_library_connection(library_dir: PathBuf) -> anyhow::Result<SqlitePool> {
        std::fs::create_dir_all(&library_dir)?;

        let options = SqliteConnectOptions::new()
            .filename(library_dir.join("library.db"))
            .synchronous(SqliteSynchronous::Normal)
            .journal_mode(SqliteJournalMode::Wal)
            .create_if_missing(true);
        let pool = SqlitePool::connect_with(options).await?;
        if let Err(_) = sqlx::migrate!("./migrations").run(&pool).await {
            error!("Failed to migrate database. Database may be corrupt.");
        }

        Ok(pool)
    }

    async fn update_library_is_set_up(&mut self, cx: &mut App) {
        let Some(pool) = &self.pool else {
            self.is_library_set_up = false;
            return;
        };

        self.is_library_set_up = matches!(
            sqlx::query("SELECT * FROM scans, tracks LIMIT 1")
                .fetch_optional(pool)
                .await,
            Ok(Some(_))
        )
    }

    pub fn erase(&mut self, cx: &mut App) {
        let Some(pool) = &self.pool else {
            return;
        };

        smol::block_on(pool.close());

        let library_dir = Self::library_dir(cx);
        remove_dir_all(&library_dir).unwrap();

        self.pool = smol::block_on(Self::open_library_connection(library_dir)).ok();
    }

    pub fn start_scan(&self, cx: &mut App) {
        let Some(pool) = &self.pool else {
            return;
        };

        let job = Rc::new(RefCell::new(StandardJob::new_transient(
            tr!("SCAN_JOB_TITLE", "Library Scan").into(),
            tr!("SCAN_JOB_DESCRIPTION", "Scanning library for music...").into(),
        )));
        let job_entity_source = job.clone();
        let job_entity = cx.new::<Jobling>(|_| job_entity_source);

        let job_clone = job_entity.clone();

        let pool = pool.clone();
        cx.spawn(async move |cx: &mut AsyncApp| {
            job_clone
                .update(cx, |_, cx| {
                    job.borrow_mut().update_job_progress(0, 0);
                    cx.notify();
                })
                .unwrap();

            // Search scans
            let mut scans_query = sqlx::query("SELECT path FROM scans").fetch(&pool);

            let mut errors_encountered = 0_usize;
            while let Some(scan) = scans_query.next().await {
                match scan {
                    Ok(row) => {
                        let path = row.get::<String, _>("path");
                        let mut entries = WalkDir::new(path);
                        loop {
                            match entries.next().await {
                                Some(Ok(entry)) => {
                                    if entry.file_type().await.unwrap().is_dir() {
                                        continue;
                                    }

                                    if let Err(e) =
                                        scan_file_into_pool(&pool, &entry.path(), cx).await
                                    {
                                        error!(
                                            "Failed to scan file: {}: {e:?}",
                                            entry.path().to_string_lossy()
                                        );
                                    }
                                }
                                Some(Err(e)) => {
                                    error!("Failed to scan file: {e:?}");
                                    errors_encountered += 1;
                                    break;
                                }
                                None => break,
                            }
                        }
                    }
                    Err(e) => {
                        error!("Failed to scan for tracks: {e:?}");
                    }
                }
            }

            job_clone
                .update(cx, |_, cx| {
                    if errors_encountered == 0 {
                        job.borrow_mut().update_job_status(
                            tr!("SCAN_JOB_COMPLETE_DESCRIPTION", "Library scan complete").into(),
                            JobStatus::Completed,
                        );
                        cx.notify();
                    } else {
                        job.borrow_mut().update_job_status(
                            trn!(
                                "SCAN_JOB_ERROR_DESCRIPTION",
                                "Library scan complete, but {{count}} error was reported",
                                "Library scan complete, but {{count}} errors were reported",
                                count = errors_encountered as isize
                            )
                            .into(),
                            JobStatus::Failed,
                        );
                        cx.notify();
                    }
                })
                .unwrap();
        })
        .detach();

        cx.update_global::<JobManager, ()>(|job_manager, cx| {
            job_manager.track_job(job_entity, cx);
        });
    }

    pub async fn scan_file(&self, path: &Path, cx: &mut AsyncApp) {
        let Some(pool) = &self.pool else {
            return;
        };

        scan_file_into_pool(pool, path, cx).await.unwrap();
    }

    pub async fn get_scan_directories(&self) -> Vec<String> {
        let Some(pool) = &self.pool else {
            return Vec::new();
        };

        let mut scans = Vec::new();
        let mut scans_query = sqlx::query("SELECT path FROM scans").fetch(pool);
        while let Some(Ok(scan)) = scans_query.next().await {
            let path = scan.get::<String, _>("path");
            scans.push(path)
        }
        scans
    }

    pub async fn set_scan_directories(
        &mut self,
        paths: &Vec<String>,
        cx: &mut App,
    ) -> Result<(), Error> {
        let Some(pool) = &self.pool else {
            return Ok(());
        };

        let mut transaction = pool.begin().await?;

        sqlx::query("DELETE FROM scans")
            .execute(&mut *transaction)
            .await?;

        for path in paths {
            sqlx::query("INSERT INTO scans(path) VALUES(?)")
                .bind(path)
                .execute(&mut *transaction)
                .await?;
        }

        transaction.commit().await?;

        if paths.is_empty() {
            self.update_library_is_set_up(cx).await;
        } else {
            // We are guaranteed to have a library that is set up
            self.is_library_set_up = true;
        }
        Ok(())
    }

    pub fn query_all_tracks<'this, 'future: 'this>(
        &'this self,
    ) -> impl Future<Output = anyhow::Result<DatabaseQuery<Track>>> + 'future {
        DatabaseQuery::new(
            self.pool.clone(),
            "SELECT
                 tracks.id as id,
                 tracks.url as url,
                 tracks.name as name,
                 album.name as album,
                 artist.name as artist,
                 tracks.track as track,
                 tracks.disc as disc
             FROM tracks
                 LEFT JOIN artist ON tracks.artist = artist.id
                 LEFT JOIN album ON tracks.album = album.id
             ORDER BY tracks.name"
                .to_string(),
            Default::default(),
        )
    }

    pub fn query_all_albums<'this, 'future: 'this>(
        &'this self,
    ) -> impl Future<Output = anyhow::Result<DatabaseQuery<Album>>> + 'future {
        DatabaseQuery::new(
            self.pool.clone(),
            "SELECT
                 album.id as id,
                 album.name as name,
                 art.image as image,
                 art.mime_type as image_mime_type
             FROM (
                -- Get the first track for each album
                SELECT album.*, coalesce(album.image_hash, tracks.image_hash) AS coalesced_hash
                    FROM album, tracks
                    WHERE
                        tracks.id = (SELECT id FROM tracks WHERE tracks.album = album.id ORDER BY tracks.track LIMIT 1)
                ) album
                LEFT JOIN art ON album.coalesced_hash = art.hash
             ORDER BY album.name"
                .to_string(),
            Default::default(),
        )
    }

    pub fn query_all_artists<'this, 'future: 'this>(
        &'this self,
    ) -> impl Future<Output = anyhow::Result<DatabaseQuery<Artist>>> + 'future {
        DatabaseQuery::new(
            self.pool.clone(),
            "SELECT
                 artist.id as id,
                 artist.name as name,
                 art.image as image,
                 art.mime_type as image_mime_type
             FROM artist
                LEFT JOIN art ON artist.image_hash = art.hash
             ORDER BY artist.name"
                .to_string(),
            Default::default(),
        )
    }

    pub fn query_album_tracks<'this, 'future: 'this>(
        &'this self,
        album_id: u32,
    ) -> impl Future<Output = anyhow::Result<DatabaseQuery<Track>>> + 'future {
        let mut args = SqliteArguments::<'static>::default();
        args.add(album_id).unwrap();
        DatabaseQuery::new(
            self.pool.clone(),
            "SELECT
                 tracks.id as id,
                 tracks.url as url,
                 tracks.name as name,
                 album.name as album,
                 artist.name as artist,
                 tracks.track as track,
                 tracks.disc as disc
             FROM tracks
                 LEFT JOIN artist ON tracks.artist = artist.id
                 LEFT JOIN album ON tracks.album = album.id
             WHERE tracks.album = ?
             ORDER BY tracks.disc, tracks.track"
                .to_string(),
            args,
        )
    }

    pub fn query_artist_tracks<'this, 'future: 'this>(
        &'this self,
        artist_id: u32,
    ) -> impl Future<Output = anyhow::Result<DatabaseQuery<Track>>> + 'future {
        let mut args = SqliteArguments::<'static>::default();
        args.add(artist_id).unwrap();
        DatabaseQuery::new(
            self.pool.clone(),
            "SELECT
                 tracks.id as id,
                 tracks.url as url,
                 tracks.name as name,
                 album.name as album,
                 artist.name as artist,
                 tracks.track as track,
                 tracks.disc as disc
             FROM tracks
                 LEFT JOIN artist ON tracks.artist = artist.id
                 LEFT JOIN album ON tracks.album = album.id
             WHERE tracks.artist = ?
             ORDER BY album.name, tracks.disc, tracks.track"
                .to_string(),
            args,
        )
    }
}

async fn scan_file_into_pool(
    pool: &SqlitePool,
    path: &Path,
    cx: &mut AsyncApp,
) -> anyhow::Result<()> {
    let url = Url::from_file_path(path).unwrap();

    let file_metadata = metadata(path)?;
    let modified_date = file_metadata
        .modified()?
        .duration_since(SystemTime::UNIX_EPOCH)?
        .as_secs_f64();

    // Find out if this file exists in the database
    let have_tracked_entry =
        sqlx::query("SELECT * FROM tracks WHERE url = ? AND file_modified_date = ? LIMIT 1")
            .bind(url.as_str())
            .bind(modified_date)
            .fetch_optional(pool)
            .await?;
    if have_tracked_entry.is_none() {
        // The file has changed, so update the metadata forcibly
        let audio_metadata = SymphoniaEngine::audio_metadata(url.clone()).await?;

        let artist_id = if let Some(artist) = audio_metadata.artist {
            let existing_artist = sqlx::query("SELECT id FROM artist WHERE name = ? LIMIT 1")
                .bind(&artist)
                .fetch_optional(pool)
                .await?
                .map(|row| row.get::<i64, _>("id"));

            if existing_artist.is_none() {
                sqlx::query(
                    "INSERT INTO artist(name) VALUES (?) ON CONFLICT DO NOTHING RETURNING id",
                )
                .bind(&artist)
                .fetch_optional(pool)
                .await?
                .map(|row| row.get::<i64, _>("id"))
            } else {
                existing_artist
            }
        } else {
            None
        };

        let album_id = if let Some(album) = audio_metadata.album {
            let existing_album = sqlx::query("SELECT id FROM album WHERE name = ? LIMIT 1")
                .bind(&album)
                .fetch_optional(pool)
                .await?
                .map(|row| row.get::<i64, _>("id"));

            if existing_album.is_none() {
                sqlx::query(
                    "INSERT INTO album(name) VALUES (?) ON CONFLICT DO NOTHING RETURNING id",
                )
                .bind(&album)
                .fetch_optional(pool)
                .await?
                .map(|row| row.get::<i64, _>("id"))
            } else {
                existing_album
            }
        } else {
            None
        };

        let art_hash = if let Some(album_cover) = audio_metadata.album_cover {
            let hash = Sha256::digest(&*album_cover.backing_store);
            let hash = format!("{hash:X}");

            sqlx::query(
                "INSERT INTO art(hash, image, mime_type)
                        VALUES (?, ?, ?)
                        ON CONFLICT DO NOTHING",
            )
            .bind(&hash)
            .bind(&*album_cover.backing_store)
            .bind(&album_cover.mime_type)
            .execute(pool)
            .await?;

            Some(hash)
        } else {
            None
        };

        sqlx::query(
            "INSERT INTO tracks(url, name, artist, album, file_modified_date, track, image_hash)
                VALUES(?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT DO
                    UPDATE SET name=?, artist=?, album=?, file_modified_date=?, track=?, image_hash=?",
        )
        // VALUES
        .bind(url.as_str())
        .bind(&audio_metadata.title)
        .bind(artist_id)
        .bind(album_id)
        .bind(modified_date)
        .bind(audio_metadata.track_number)
        .bind(&art_hash)
        // ON CONFLICT DO UPDATE SET
        .bind(&audio_metadata.title)
        .bind(artist_id)
        .bind(album_id)
        .bind(modified_date)
        .bind(audio_metadata.track_number)
        .bind(&art_hash)
        .execute(pool)
        .await?;

        cx.update_global::<Database, ()>(|_, _| ()).unwrap();
    }

    Ok(())
}

impl Global for Database {}
