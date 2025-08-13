use gpui::http_client::Url;
use gpui::{actions, App, KeyBinding, PathPromptOptions};
use gpui::AppContext;
use lthebeat::audio_processing::audio_controller::GlobalAudioController;

actions!(thebeat, [OpenFileAction, OpenUrlAction]);

pub fn register_actions(cx: &mut App) {
    cx.on_action(open_file);
    // cx.on_action(open_url);
    cx.bind_keys([
        KeyBinding::new("secondary-o", OpenFileAction, None),
        KeyBinding::new("secondary-shift-o", OpenUrlAction, None)
    ])
}

fn open_file(_: &OpenFileAction, cx: &mut App) {
    let future = cx.prompt_for_paths(PathPromptOptions {
        files: true,
        directories: false,
        multiple: false,
    });
    cx.spawn(async |cx| {
        let result = future.await;
        cx.read_global::<GlobalAudioController, ()>(|global_audio_controller: &GlobalAudioController, _| {
            if let Ok(Ok(Some(x))) = result {
                global_audio_controller.audio_controller.play_url(Url::from_file_path(x.first().unwrap().as_path()).unwrap())
            }
        }).unwrap();
    }).detach()
}
