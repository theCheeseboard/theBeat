use cntp_i18n::{I18N_MANAGER, tr_load};
use gpui::{App, BorrowAppContext};
use lthebeat::audio_processing::input_engines::EngineManager;
use std::fmt::Debug;
use std::str::FromStr;
use url::Url;

#[cfg(target_os = "linux")]
pub mod cdio_paranoia_engine;

#[cfg(target_os = "linux")]
pub mod cd_view;

#[cfg(target_os = "linux")]
mod watch_storage;

pub fn init(cx: &mut App) {
    I18N_MANAGER.load_source(tr_load!());

    cfg_select! {
        target_os = "linux" => {
            cx.update_global::<EngineManager, _>(|manager, _| {
                manager.register_factory(cdio_paranoia_engine::CdioParanoiaFactory::default());
            });
            cx.set_global(cdio_paranoia_engine::cdio_manager::CdioManager::new());

            watch_storage::watch_storage(cx);
        }
        _ => {
            tracing::warn!("cdio-paranoia is only supported on Linux");
        }
    }
}

pub fn track_url<T: TryInto<u64>>(block_device: &str, track: T) -> Url
where
    <T as TryInto<u64>>::Error: Debug,
{
    Url::from_str(&format!(
        "cd://{block_device}?track={}",
        track.try_into().unwrap()
    ))
    .unwrap()
}
