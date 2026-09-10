use cntp_i18n::{tr_load, I18N_MANAGER};
use gpui::{App, BorrowAppContext};
use lthebeat::audio_processing::input_engines::EngineManager;

#[cfg(target_os = "linux")]
pub mod cdio_paranoia_engine;

#[cfg(target_os = "linux")]
mod watch_storage;
pub mod cd_view;

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