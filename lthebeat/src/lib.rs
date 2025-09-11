use crate::platform::setup_platform;
use cntp_i18n::{I18N_MANAGER, tr_load};
use gpui::App;

pub mod audio_library;
pub mod audio_processing;
mod cyclic_cursor_vec;
mod platform;
pub mod play_queue;

pub fn setup_libthebeat(cx: &mut App) {
    I18N_MANAGER.write().unwrap().load_source(tr_load!());
    setup_platform(cx);
}
