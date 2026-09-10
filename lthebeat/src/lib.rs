use crate::audio_processing::input_engines::EngineManager;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaFactory;
use crate::other_sources::OtherSourcesManager;
use crate::platform::setup_platform;
use cntp_i18n::{I18N_MANAGER, tr_load};
use gpui::{App, BorrowAppContext};

pub mod audio_library;
pub mod audio_processing;
mod cyclic_cursor_vec;
mod platform;
pub mod play_queue;
pub mod other_sources;

pub fn setup_libthebeat(cx: &mut App) {
    I18N_MANAGER.load_source(tr_load!());
    setup_platform(cx);

    cx.set_global(OtherSourcesManager::new());
    cx.set_global(EngineManager::new());
    
    cx.update_global::<EngineManager, _>(|manager, _| {
        manager.register_factory(SymphoniaFactory::default());
    });
}
