mod playlist_library;

use crate::views::albums_view::AlbumsView;
use crate::views::playlists_view::playlist_library::PlaylistLibrary;
use contemporary::components::pager::lift_animation::LiftAnimation;
use contemporary::components::pager::pager;
use gpui::{AppContext, Context, Entity, IntoElement, ParentElement, Render, Styled, Window, div};
use lthebeat::audio_library::album::Album;
use lthebeat::audio_library::playlist::Playlist;

pub struct PlaylistsView {
    playlist_library: Entity<PlaylistLibrary>,
    in_playlist_view: bool,
}

impl PlaylistsView {
    pub fn new(cx: &mut Context<Self>) -> Self {
        let on_playlist_click_listener =
            cx.listener(|this: &mut PlaylistsView, playlist: &Entity<Playlist>, _, cx| {
                // let back_click_listener = cx.listener(|this: &mut PlaylistsView, _, _, cx| {
                //     this.in_album_view = false;
                //     cx.notify();
                // });
                //
                // this.individual_album_view = Some(crate::views::albums_view::individual_album_view::IndividualAlbumView::new(
                //     album.clone(),
                //     Box::new(back_click_listener),
                //     cx,
                // ));
                // this.in_album_view = true;
                // cx.notify();
            });

        Self {
            playlist_library: cx
                .new(|cx| PlaylistLibrary::new(Box::new(on_playlist_click_listener), cx)),
            in_playlist_view: false,
        }
    }
}

impl Render for PlaylistsView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        div().size_full().child(
            pager(
                "playlists-view-pager",
                if self.in_playlist_view { 1 } else { 0 },
            )
            .w_full()
            .h_full()
            .animation(LiftAnimation::new())
            .page(self.playlist_library.clone().into_any_element()), // .when_some(
                                                                     //     self.individual_album_view.as_ref(),
                                                                     //     |pager, individual_album_view| {
                                                                     //         pager.page(individual_album_view.clone().into_any_element())
                                                                     //     },
                                                                     // ),
        )
    }
}
