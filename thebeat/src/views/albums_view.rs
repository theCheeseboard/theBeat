use crate::database_setup::database_setup_interstitial::database_setup_interstitial;
use crate::views::albums_view::album_library::AlbumLibrary;
use crate::views::albums_view::individual_album_view::IndividualAlbumView;
use contemporary::components::pager::lift_animation::LiftAnimation;
use contemporary::components::pager::pager;
use contemporary::styling::theme::Theme;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, Context, Entity, InteractiveElement, IntoElement, ParentElement, Render,
    StatefulInteractiveElement, Styled, Window, div,
};
use lthebeat::audio_library::album::Album;
use lthebeat::audio_library::database::Database;
use smol::io::AsyncReadExt;
use std::ops::Deref;

mod album_library;
mod individual_album_view;

pub struct AlbumsView {
    album_library: Entity<AlbumLibrary>,
    individual_album_view: Option<Entity<IndividualAlbumView>>,
    in_album_view: bool,
}

impl AlbumsView {
    pub fn new(cx: &mut App) -> Entity<Self> {
        cx.new(|cx| {
            let on_album_click_listener =
                cx.listener(|this: &mut AlbumsView, album: &Entity<Album>, _, cx| {
                    let back_click_listener = cx.listener(|this: &mut AlbumsView, _, _, cx| {
                        this.in_album_view = false;
                        cx.notify();
                    });

                    this.individual_album_view = Some(IndividualAlbumView::new(
                        album.clone(),
                        Box::new(back_click_listener),
                        cx,
                    ));
                    this.in_album_view = true;
                    cx.notify();
                });

            AlbumsView {
                album_library: AlbumLibrary::new(Box::new(on_album_click_listener), cx),
                individual_album_view: None,
                in_album_view: false,
            }
        })
    }
}

impl Render for AlbumsView {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let database = cx.global::<Database>();

        div().size_full().when_else(
            database.is_library_set_up,
            |david| {
                david.child(
                    pager("albums-view-pager", if self.in_album_view { 1 } else { 0 })
                        .w_full()
                        .h_full()
                        .animation(LiftAnimation::new())
                        .page(self.album_library.clone().into_any_element())
                        .when_some(
                            self.individual_album_view.as_ref(),
                            |pager, individual_album_view| {
                                pager.page(individual_album_view.clone().into_any_element())
                            },
                        ),
                )
            },
            |david| david.child(database_setup_interstitial()),
        )
    }
}
