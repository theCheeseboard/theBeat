use crate::audio_library::album::Album;
use crate::audio_library::database_query::DatabaseQuery;
use crate::audio_library::track::Track;
use crate::audio_processing::audio_metadata;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use async_walkdir::{Filtering, WalkDir};
use cntp_i18n::tr;
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
use sqlx::Arguments;
use sqlx::sqlite::{
    SqliteArguments, SqliteConnectOptions, SqliteJournalMode, SqlitePoolOptions, SqliteSynchronous,
};
use sqlx::{Executor, Row, SqlitePool};
use std::cell::RefCell;
use std::fs::metadata;
use std::path::Path;
use std::ptr::read;
use std::rc::Rc;
use std::time::{Duration, SystemTime};
use tracing::error;
use url::Url;

pub struct Database {
    pool: SqlitePool,
}

impl Database {
    pub async fn new(cx: &mut App) -> anyhow::Result<Self> {
        let details = cx.global::<Details>();

        let directories = details.standard_dirs().unwrap();
        let data_dir = directories.data_dir();

        let library_dir = data_dir.join("library");
        std::fs::create_dir_all(&library_dir)?;

        let options = SqliteConnectOptions::new()
            .filename(library_dir.join("library.db"))
            .synchronous(SqliteSynchronous::Normal)
            .journal_mode(SqliteJournalMode::Wal)
            .create_if_missing(true);
        let pool = SqlitePool::connect_with(options).await?;
        sqlx::migrate!("./migrations").run(&pool).await?;
        Ok(Self { pool })
    }

    pub fn start_scan(&self, cx: &mut App) {
        let job = Rc::new(RefCell::new(StandardJob::new_transient(
            tr!("SCAN_JOB_TITLE", "Library Scan").into(),
            tr!("SCAN_JOB_DESCRIPTION", "Scanning library for music...").into(),
        )));
        let job_entity_source = job.clone();
        let job_entity = cx.new::<Jobling>(|_| job_entity_source);

        let job_clone = job_entity.clone();

        let pool = self.pool.clone();
        cx.spawn(async move |cx: &mut AsyncApp| {
            job_clone
                .update(cx, |_, cx| {
                    job.borrow_mut().update_job_progress(0, 0);
                    cx.notify();
                })
                .unwrap();
            if let Some(user_dir) = UserDirs::new() {
                // Get the Music directory
                if let Some(music_dir) = user_dir.audio_dir() {
                    let mut entries = WalkDir::new(music_dir);

                    loop {
                        match entries.next().await {
                            Some(Ok(entry)) => {
                                if entry.file_type().await.unwrap().is_dir() {
                                    continue;
                                }

                                if let Err(e) = scan_file_into_pool(&pool, &entry.path(), cx).await
                                {
                                    error!(
                                        "Failed to scan file: {}: {e:?}",
                                        entry.path().to_string_lossy()
                                    );
                                }
                            }
                            Some(Err(e)) => {
                                eprintln!("error: {}", e);
                                break;
                            }
                            None => break,
                        }
                    }
                }

                job_clone
                    .update(cx, |_, cx| {
                        job.borrow_mut().update_job_status(
                            tr!("SCAN_JOB_COMPLETE_DESCRIPTION", "Library scan complete").into(),
                            JobStatus::Completed,
                        );
                        cx.notify();
                    })
                    .unwrap();
            }
        })
        .detach();

        cx.update_global::<JobManager, ()>(|job_manager, cx| {
            job_manager.track_job(job_entity, cx);
        });
    }

    pub async fn scan_file(&self, path: &Path, cx: &mut AsyncApp) {
        scan_file_into_pool(&self.pool, path, cx).await.unwrap();
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
                        tracks.album = album.id AND
                        tracks.id = (SELECT id FROM tracks WHERE tracks.album = album.id ORDER BY tracks.track LIMIT 1)
                ) album
                LEFT JOIN art ON album.coalesced_hash = art.hash
             ORDER BY album.name"
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
