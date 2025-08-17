use gpui::AppContext;
use gpui::http_client::Url;
use gpui::{App, AsyncApp, KeyBinding, PathPromptOptions, actions};
use lthebeat::audio_processing::audio_controller::GlobalAudioController;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;

actions!(thebeat, [OpenFileAction, OpenUrlAction]);

pub fn register_actions(cx: &mut App) {
    cx.on_action(open_file);
    // cx.on_action(open_url);
    cx.bind_keys([
        KeyBinding::new("secondary-o", OpenFileAction, None),
        KeyBinding::new("secondary-shift-o", OpenUrlAction, None),
    ])
}

fn open_file(_: &OpenFileAction, cx: &mut App) {
    let future = cx.prompt_for_paths(PathPromptOptions {
        files: true,
        directories: false,
        multiple: false,
    });
    cx.spawn(async |cx: &mut AsyncApp| {
        let result = future.await;
        cx.update_global::<PlayQueue, ()>(|play_queue: &mut PlayQueue, cx| {
            if let Ok(Ok(Some(x))) = result {
                let item = MediaItem::new(
                    Url::from_file_path(x.first().unwrap().as_path()).unwrap(),
                    cx,
                );
                play_queue.add_item(item);
            }
        })
        .unwrap();
    })
    .detach()
}
