use crate::track_listing::track_listing;
use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::Theme;
use gpui::private::anyhow;
use gpui::{
    App, AppContext, AsyncApp, Context, Entity, IntoElement, ParentElement, Render, Styled,
    Subscription, WeakEntity, Window, div, px, uniform_list,
};
use lthebeat::audio_library::album::Album;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::track::Track;
use std::cell::RefCell;
use std::ops::Deref;
use std::rc::Rc;

pub struct IndividualAlbumView {
    album_name: String,
    database_subscription: Subscription,
    tracks_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>>,
    on_back_clicked: Rc<Box<dyn Fn(&(), &mut Window, &mut App) + 'static>>,
}

impl IndividualAlbumView {
    pub fn new(
        album: Entity<Album>,
        on_back_clicked: Box<dyn Fn(&(), &mut Window, &mut App) + 'static>,
        cx: &mut App,
    ) -> Entity<Self> {
        let Album::Ok {
            id: album_id,
            name: album_name,
            art: album_art,
        } = album.read(cx).clone()
        else {
            panic!("Album is not Ok");
        };

        cx.new(|cx| {
            let database_subscription =
                cx.observe_global::<Database>(move |_, cx: &mut Context<IndividualAlbumView>| {
                    update_tracks_query(album_id, cx);
                });

            update_tracks_query(album_id, cx);

            IndividualAlbumView {
                album_name: album_name.unwrap_or("Album".to_string()),
                database_subscription,
                tracks_query: None,
                on_back_clicked: Rc::new(on_back_clicked),
            }
        })
    }
}

impl Render for IndividualAlbumView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        let back_clicked_handler = self.on_back_clicked.clone();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("tracks-grandstand")
                    .text(tr!(
                        "INDIVIDUAL_ALBUM_TITLE",
                        "Tracks in {{album}}",
                        album:quote = self.album_name,
                    ))
                    .pt(px(36.))
                    .on_back_click(move |_, window, cx| back_clicked_handler(&(), window, cx)),
            )
            .child(match self.tracks_query.as_mut() {
                Some(tracks_query) => match tracks_query.borrow().deref() {
                    Ok(_) => div()
                        .flex_grow()
                        .w_full()
                        .child(track_listing(tracks_query.clone()))
                        .into_any_element(),
                    Err(_) => interstitial()
                        .w_full()
                        .h_full()
                        .icon("media-album-cover".into())
                        .title(tr!("LIBRARY_ALBUM_ERROR", "Unable to load album").into())
                        .message(tr!("LIBRARY_CORRUPT_ERROR_MESSAGE",).into())
                        .into_any_element(),
                },
                _ => div().child(spinner()).into_any_element(),
            })
    }
}

fn update_tracks_query(album_id: u32, cx: &mut Context<IndividualAlbumView>) {
    let database = cx.global::<Database>();
    let query = database.query_album_tracks(album_id);

    cx.spawn(
        async move |tracks_view: WeakEntity<IndividualAlbumView>, cx: &mut AsyncApp| {
            let tracks_query = query.await;
            tracks_view
                .update(cx, |tracks_view, cx| {
                    tracks_view.tracks_query = Some(Rc::new(RefCell::new(tracks_query)));
                    cx.notify();
                })
                .unwrap();
        },
    )
    .detach();
}
