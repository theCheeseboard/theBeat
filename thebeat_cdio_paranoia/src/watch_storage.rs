use std::collections::HashMap;
use std::ffi::CStr;
use std::time::Duration;
use gpui::{App, AppContext, AsyncApp, Global};
use smol::stream::StreamExt;
use tracing::{debug, info, warn};
use udisks2::Client;
use udisks2::zbus::zvariant::OwnedObjectPath;
use lthebeat::other_sources::OtherSourcesManager;
use crate::cd_view::CdSource;

#[derive(Default)]
struct StorageManager {
    block_devices: HashMap<OwnedObjectPath, ()>,
}

impl Global for StorageManager {}

impl StorageManager {
    pub fn clear(&mut self) {
        self.block_devices.clear();
    }
}

pub fn watch_storage(cx: &mut App) {
    cx.set_global(StorageManager::default());

    cx.spawn(async move |cx: &mut AsyncApp| {
        loop {
            if let Err(e) = watch_loop(cx).await {
                warn!("udisks error: {e}");
            }

            cx.update_global::<StorageManager, _>(|storage_manager, cx| {
                storage_manager.clear();
            });

            warn!("Retrying udisks in 5 seconds");
            cx.background_executor().timer(Duration::from_secs(5)).await;
        }
    }).detach()
}

async fn watch_loop(cx: &mut AsyncApp) -> udisks2::Result<()> {
    let client = Client::new().await?;
    let manager = client.manager();
    let block_devices = manager.get_block_devices(Default::default()).await?;

    for block_device in block_devices {
        watch_block_device(&client, block_device, cx).await;
    }

    let mut interfaces_added = client.object_manager().receive_interfaces_added().await?;

    while let Some(interface) = interfaces_added.next().await {
        if let Ok(args) = interface.args() {
            if args.interfaces_and_properties().contains_key("org.freedesktop.UDisks2.Block") {
                watch_block_device(&client, args.object_path().to_owned().into(), cx).await;
            }
        }
    }

    Ok(())
}

async fn watch_block_device(client: &Client, block_device: OwnedObjectPath, cx: &mut AsyncApp) {
    let client = client.clone();
    cx.spawn(async move |cx: &mut AsyncApp| {
        let Ok(object) = client.object(block_device);

        let Ok(block) = object.block().await else {
            return;
        };

        let Ok(drive_path) = block.drive().await else {
            return;
        };

        let Ok(drive_object) = client.object(drive_path);

        let Ok(drive) = drive_object.drive().await else {
            return;
        };

        if !drive.optical().await.is_ok_and(|optical| optical) {
            return;
        }

        let Ok(block_device) = block.device().await else {
            return;
        };

        let Ok(block_device) = CStr::from_bytes_with_nul(&block_device) else {
            return;
        };

        let Ok(block_device) = block_device.to_str() else {
            return;
        };

        let drive_vendor = drive.vendor().await;
        let drive_model = drive.model().await;

        let drive_name = if let Ok(ref drive_vendor) = drive_vendor && let Ok(ref drive_model) = drive_model {
            format!("{drive_vendor} {drive_model}")
        } else if let Ok(drive_vendor) = drive_vendor {
            drive_vendor
        } else if let Ok(drive_model) = drive_model {
            drive_model
        } else {
            "CD".into()
        };

        let mut current_tracks = drive.optical_num_audio_tracks().await.unwrap_or(0);

        info!("Started watching block device at {block_device}. Current tracks: {current_tracks}");

        let mut source_uuid = None;
        if current_tracks > 0 {
            // Create the CD object
            source_uuid = Some(cx.update_global::<OtherSourcesManager, _>(|other_sources_manager, cx| {
                other_sources_manager.push_source(Box::new(CdSource::new(&client, &block_device, current_tracks, &drive_name, cx)))
            }))
        }

        let mut tracks_changed = drive.receive_optical_num_audio_tracks_changed().await;
        while let Some(new_tracks) = tracks_changed.next().await {
            if let Ok(new_tracks) = new_tracks.get().await {
                info!("{block_device}: number of tracks changed. Old: {current_tracks}, new: {new_tracks}");

                if let Some(source_uuid) = source_uuid.take() {
                    cx.update_global::<OtherSourcesManager, _>(|other_sources_manager, _| {
                        other_sources_manager.remove_source(source_uuid);
                    })
                }
                if new_tracks > 0 {
                    // Create the CD object
                    source_uuid = Some(cx.update_global::<OtherSourcesManager, _>(|other_sources_manager, cx| {
                        other_sources_manager.push_source(Box::new(CdSource::new(&client, &block_device, new_tracks, &drive_name, cx)))
                    }))
                }

                current_tracks = new_tracks;
            }
        }

        if let Some(source_uuid) = source_uuid {
            cx.update_global::<OtherSourcesManager, _>(|other_sources_manager, _| {
                other_sources_manager.remove_source(source_uuid);
            })
        }

        info!("Stopped watching block device at {block_device}")
    }).detach()
}