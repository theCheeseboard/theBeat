use gpui::{App, BorrowAppContext};
use lthebeat::audio_processing::input_engines::EngineManager;

#[cfg(target_os = "linux")]
pub mod cdio_paranoia_engine;

pub fn init(cx: &mut App) {
    cfg_select! {
        target_os = "linux" => {
            cx.update_global::<EngineManager, _>(|manager, _| {
                manager.register_factory(cdio_paranoia_engine::CdioParanoiaFactory::default());
            });
            cx.set_global(cdio_paranoia_engine::cdio_manager::CdioManager::new());
        }
        _ => {
            warn!("cdio-paranoia is only supported on Linux");
        }
    }
}