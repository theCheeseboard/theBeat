use crate::database_setup::database_setup_interstitial::database_setup_interstitial;
use crate::views::artists_view::artists_library::ArtistsLibrary;
use crate::views::artists_view::individual_artist_view::IndividualArtistView;
use contemporary::components::pager::lift_animation::LiftAnimation;
use contemporary::components::pager::pager;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, ClickEvent, Context, Entity, IntoElement, ParentElement, Render, Styled,
    Window, div,
};
use lthebeat::audio_library::artist::Artist;
use lthebeat::audio_library::database::Database;
use std::rc::Rc;

pub mod artists_library;
mod individual_artist_view;

pub struct ArtistsView {
    artists_library: Entity<ArtistsLibrary>,
    individual_artists_view: Option<Entity<IndividualArtistView>>,
    on_setup_button_click: Rc<Box<dyn Fn(&ClickEvent, &mut Window, &mut App) + 'static>>,
    in_artists_view: bool,
}

impl ArtistsView {
    pub fn new(
        on_setup_button_click: Rc<Box<dyn Fn(&ClickEvent, &mut Window, &mut App) + 'static>>,
        cx: &mut App,
    ) -> Entity<Self> {
        cx.new(|cx| {
            let on_artists_click_listener =
                cx.listener(|this: &mut ArtistsView, artists: &Entity<Artist>, _, cx| {
                    let back_click_listener = cx.listener(|this: &mut ArtistsView, _, _, cx| {
                        this.in_artists_view = false;
                        cx.notify();
                    });

                    this.individual_artists_view = Some(IndividualArtistView::new(
                        artists.clone(),
                        Box::new(back_click_listener),
                        cx,
                    ));
                    this.in_artists_view = true;
                    cx.notify();
                });

            ArtistsView {
                artists_library: ArtistsLibrary::new(Box::new(on_artists_click_listener), cx),
                individual_artists_view: None,
                on_setup_button_click,
                in_artists_view: false,
            }
        })
    }
}

impl Render for ArtistsView {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let database = cx.global::<Database>();

        div().size_full().when_else(
            database.is_library_set_up,
            |david| {
                david.child(
                    pager(
                        "artistss-view-pager",
                        if self.in_artists_view { 1 } else { 0 },
                    )
                    .w_full()
                    .h_full()
                    .animation(LiftAnimation::new())
                    .page(self.artists_library.clone().into_any_element())
                    .when_some(
                        self.individual_artists_view.as_ref(),
                        |pager, individual_artists_view| {
                            pager.page(individual_artists_view.clone().into_any_element())
                        },
                    ),
                )
            },
            |david| {
                let setup_button_click = self.on_setup_button_click.clone();
                david.child(database_setup_interstitial().on_setup_button_click(
                    move |event, window, cx| {
                        setup_button_click(event, window, cx);
                    },
                ))
            },
        )
    }
}
