use gpui::http_client::Url;
use gpui::{App, AsyncApp, BorrowAppContext, KeyBinding, PathPromptOptions, actions};
use lthebeat::audio_processing::audio_controller::AudioController;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;

actions!(
    thebeat,
    [
        OpenFileAction,
        OpenUrlAction,
        PlayPauseAction,
        SkipNextAction,
        SkipPreviousAction,
        DatabaseSetupAction,
    ]
);

pub fn register_actions(cx: &mut App) {
    cx.on_action(open_file);
    cx.on_action(play_pause);
    cx.on_action(skip_next);
    cx.on_action(skip_previous);
    cx.bind_keys([
        KeyBinding::new("secondary-o", OpenFileAction, None),
        KeyBinding::new("secondary-shift-o", OpenUrlAction, None),
        KeyBinding::new("space", PlayPauseAction, None),
        KeyBinding::new("shift-right", SkipNextAction, None),
        KeyBinding::new("shift-left", SkipPreviousAction, None),
    ])
}

fn open_file(_: &OpenFileAction, cx: &mut App) {
    let future = cx.prompt_for_paths(PathPromptOptions {
        files: true,
        directories: false,
        multiple: true,
        prompt: None,
    });
    cx.spawn(async |cx: &mut AsyncApp| {
        let result = future.await;
        cx.update_global::<PlayQueue, ()>(|play_queue: &mut PlayQueue, cx| {
            if let Ok(Ok(Some(paths))) = result {
                for path in paths {
                    let item = MediaItem::new(Url::from_file_path(path.as_path()).unwrap(), cx);
                    play_queue.add_item(item, cx);
                }
            }
        })
        .unwrap();
    })
    .detach()
}

fn skip_next(_: &SkipNextAction, cx: &mut App) {
    let play_queue = cx.global_mut::<PlayQueue>();
    play_queue.skip_next();
}

fn skip_previous(_: &SkipPreviousAction, cx: &mut App) {
    let play_queue = cx.global_mut::<PlayQueue>();
    play_queue.skip_previous();
}

fn play_pause(_: &PlayPauseAction, cx: &mut App) {
    cx.update_global::<AudioController, ()>(|audio_controller: &mut AudioController, _| {
        audio_controller.play_pause();
    })
}
