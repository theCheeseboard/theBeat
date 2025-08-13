use std::sync::mpsc;

pub fn mpsc_returnable<TMessage>() {
    let (tx, rx) = mpsc::channel::<TMessage>();


}