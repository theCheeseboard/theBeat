use std::{env, path::PathBuf};

fn main() {
    // Rebuild if migrations change
    println!("cargo:rerun-if-changed=migrations");

    // Initialise i18n
    let path: PathBuf = env::var("CARGO_MANIFEST_DIR")
        .expect("CARGO_MANIFEST_DIR is not set")
        .into();

    cntp_i18n_gen::generate_default(&path);
}
