use cntp_i18n::{tr_load, I18N_MANAGER};

pub mod audio_library;
pub mod audio_processing;
mod cyclic_cursor_vec;
pub mod play_queue;

pub fn install_translations() {
    I18N_MANAGER.write().unwrap().load_source(tr_load!());
}