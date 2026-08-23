use crate::track_listing::track_listing;
use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::{Theme, ThemeStorage};
use gpui::private::anyhow;
use gpui::{
    Along, Anchor, AnyElement, App, AppContext, AsyncApp, Axis, Background, BorderStyle, Bounds,
    Context, Corners, Edges, EdgesRefinement, Element, ElementId, Entity, GlobalElementId,
    InspectorElementId, IntoElement, LayoutId, LinearColorStop, ParentElement, Pixels, Refineable,
    Render, RenderImage, Rgba, Size, Style, StyleRefinement, Styled, Subscription, WeakEntity,
    Window, div, linear_color_stop, linear_gradient, px, quad, rgb, transparent_black,
    uniform_list,
};
use lthebeat::audio_library::artist::Artist;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::playlist::Playlist;
use lthebeat::audio_library::track::Track;
use lthebeat::audio_processing::audio_metadata::Art;
use std::cell::RefCell;
use std::ops::Deref;
use std::panic::Location;
use std::rc::Rc;
use std::sync::Arc;

pub struct IndividualPlaylistView {
    playlist_name: String,
    database_subscription: Subscription,
    tracks_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>>,
    on_back_clicked: Rc<Box<dyn Fn(&(), &mut Window, &mut App) + 'static>>,
}

impl IndividualPlaylistView {
    pub fn new(
        playlists: Entity<Playlist>,
        on_back_clicked: Box<dyn Fn(&(), &mut Window, &mut App) + 'static>,
        cx: &mut App,
    ) -> Entity<Self> {
        let Playlist::Ok {
            name: playlist_name,
            ..
        } = playlists.read(cx).clone()
        else {
            panic!("Playlists is not Ok");
        };

        cx.new(|cx| {
            let database_subscription = cx.observe_global::<Database>({
                let playlists = playlists.clone();
                move |_, cx: &mut Context<IndividualPlaylistView>| {
                    update_tracks_query(playlists.clone(), cx);
                }
            });

            update_tracks_query(playlists, cx);

            IndividualPlaylistView {
                playlist_name,
                database_subscription,
                tracks_query: None,
                on_back_clicked: Rc::new(on_back_clicked),
            }
        })
    }
}

impl Render for IndividualPlaylistView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.theme();

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
                        "INDIVIDUAL_PLAYLISTS_TITLE",
                        "Tracks in {{playlist}}",
                        playlist:quote = self.playlist_name,
                    ))
                    .pt(px(36.))
                    .on_back_click(move |_, window, cx| back_clicked_handler(&(), window, cx)),
            )
            .child(match self.tracks_query.as_mut() {
                Some(tracks_query) => match tracks_query.borrow().deref() {
                    Ok(_) => div()
                        .flex_grow(1.)
                        .w_full()
                        .child(track_listing(tracks_query.clone()))
                        .into_any_element(),
                    Err(_) => interstitial()
                        .w_full()
                        .h_full()
                        .icon("view-media-playlist")
                        .title(tr!("LIBRARY_PLAYLISTS_ERROR", "Unable to load playlists"))
                        .message(tr!("LIBRARY_CORRUPT_ERROR_MESSAGE"))
                        .into_any_element(),
                },
                _ => div().child(spinner()).into_any_element(),
            })
    }
}

fn update_tracks_query(playlists: Entity<Playlist>, cx: &mut Context<IndividualPlaylistView>) {
    let playlists = playlists.read(cx).clone();

    cx.spawn(
        async move |playlists_view: WeakEntity<IndividualPlaylistView>, cx: &mut AsyncApp| {
            let tracks_query = playlists.read_tracks().await;
            playlists_view
                .update(cx, |playlists_view, cx| {
                    playlists_view.tracks_query = Some(Rc::new(RefCell::new(tracks_query)));
                    cx.notify();
                })
                .unwrap();
        },
    )
    .detach();
}
