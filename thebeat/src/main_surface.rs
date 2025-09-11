use crate::OpenFileAction;
use crate::OpenUrlAction;
use crate::SkipNextAction;
use crate::SkipPreviousAction;
use crate::actions::DatabaseSetupAction;
use crate::main_surface::MainSurfaceTab::{Albums, Artists, OtherSources, Playlists, Tracks};
use crate::play_queue::play_queue;
use crate::transport_controls::TransportControls;
use crate::views::albums_view::AlbumsView;
use crate::views::artists_view::ArtistsView;
use crate::views::other_sources_view::OtherSourcesView;
use crate::views::tracks_view::TracksView;
use cntp_i18n::tr;
use contemporary::components::application_menu::ApplicationMenu;
use contemporary::components::button::button;
use contemporary::components::icon_text::icon_text;
use contemporary::components::pager::pager;
use contemporary::components::pager::slide_horizontal_animation::SlideHorizontalAnimation;
use contemporary::styling::theme::Theme;
use contemporary::surface::surface;
use gpui::{
    App, AppContext, ClickEvent, Context, Entity, InteractiveElement, IntoElement, Menu, MenuItem,
    ParentElement, Render, Styled, Window, div, px,
};
use std::rc::Rc;

pub struct MainSurface {
    application_menu: Entity<ApplicationMenu>,
    selected_tab: MainSurfaceTab,

    tracks_view: Entity<TracksView>,
    artists_view: Entity<ArtistsView>,
    albums_view: Entity<AlbumsView>,
    other_sources_view: Entity<OtherSourcesView>,

    transport_controls: Entity<TransportControls>,
}

#[derive(PartialEq)]
enum MainSurfaceTab {
    Tracks,
    Artists,
    Albums,
    Playlists,
    OtherSources,
}

impl MainSurfaceTab {
    fn index(&self) -> usize {
        match self {
            Tracks => 0,
            MainSurfaceTab::Artists => 1,
            MainSurfaceTab::Albums => 2,
            MainSurfaceTab::Playlists => 3,
            MainSurfaceTab::OtherSources => 4,
        }
    }
}

impl MainSurface {
    pub fn new(
        on_setup_button_click: Rc<Box<dyn Fn(&ClickEvent, &mut Window, &mut App) + 'static>>,
        cx: &mut App,
    ) -> Entity<MainSurface> {
        cx.new(|cx| MainSurface {
            application_menu: ApplicationMenu::new(
                cx,
                Menu {
                    name: "Application Menu".into(),
                    items: vec![
                        MenuItem::action(tr!("FILE_OPEN"), OpenFileAction),
                        MenuItem::action(tr!("FILE_OPEN_URL"), OpenUrlAction),
                        MenuItem::separator(),
                        MenuItem::action(tr!("PLAYBACK_SKIP_PREVIOUS"), SkipPreviousAction),
                        MenuItem::action(tr!("PLAYBACK_SKIP_NEXT"), SkipNextAction),
                        MenuItem::separator(),
                        MenuItem::action(tr!("FILE_DATABASE_SETUP"), DatabaseSetupAction),
                    ],
                },
            ),
            selected_tab: Tracks,
            tracks_view: TracksView::new(on_setup_button_click.clone(), cx),
            artists_view: ArtistsView::new(on_setup_button_click.clone(), cx),
            albums_view: AlbumsView::new(on_setup_button_click, cx),
            other_sources_view: OtherSourcesView::new(cx),
            transport_controls: TransportControls::new(cx),
        })
    }
}

impl Render for MainSurface {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        surface()
            .actions(
                div().occlude().flex().content_stretch().child(
                    div()
                        .flex()
                        .id("action-bar")
                        .bg(theme.button_background)
                        .rounded(theme.border_radius)
                        .gap(px(2.))
                        .content_stretch()
                        .child(
                            button("tracks-button")
                                .child(icon_text(
                                    "view-media-track".into(),
                                    tr!("TRACKS_BUTTON", "Tracks").into(),
                                ))
                                .checked_when(self.selected_tab == Tracks)
                                .on_click(cx.listener(|this, _, _, cx| {
                                    this.selected_tab = Tracks;
                                    cx.notify();
                                })),
                        )
                        .child(
                            button("artists-button")
                                .child(icon_text(
                                    "view-media-artist".into(),
                                    tr!("ARTISTS_BUTTON", "Artists").into(),
                                ))
                                .checked_when(self.selected_tab == Artists)
                                .on_click(cx.listener(|this, _, _, cx| {
                                    this.selected_tab = Artists;
                                    cx.notify();
                                })),
                        )
                        .child(
                            button("albums-button")
                                .child(icon_text(
                                    "media-album-cover".into(),
                                    tr!("ALBUMS_BUTTON", "Albums").into(),
                                ))
                                .checked_when(self.selected_tab == Albums)
                                .on_click(cx.listener(|this, _, _, cx| {
                                    this.selected_tab = Albums;
                                    cx.notify();
                                })),
                        )
                        .child(
                            button("playlists-button")
                                .child(icon_text(
                                    "view-media-playlist".into(),
                                    tr!("PLAYLISTS_BUTTON", "Playlists").into(),
                                ))
                                .checked_when(self.selected_tab == Playlists)
                                .on_click(cx.listener(|this, _, _, cx| {
                                    this.selected_tab = Playlists;
                                    cx.notify();
                                })),
                        )
                        .child(
                            button("other-sources-button")
                                .child(icon_text(
                                    "view-list-details".into(),
                                    tr!("OTHER_SOURCES_BUTTON", "Other Sources").into(),
                                ))
                                .checked_when(self.selected_tab == OtherSources)
                                .on_click(cx.listener(|this, _, _, cx| {
                                    this.selected_tab = OtherSources;
                                    cx.notify();
                                })),
                        ),
                ),
            )
            .child(
                div()
                    .flex()
                    .flex_col()
                    .gap(px(4.))
                    .w_full()
                    .h_full()
                    .child(
                        div()
                            .flex()
                            .flex_grow()
                            .gap(px(4.))
                            .child(
                                pager("main-pager", self.selected_tab.index())
                                    .flex_grow()
                                    .h_full()
                                    .animation(SlideHorizontalAnimation::new())
                                    .page(self.tracks_view.clone().into_any_element())
                                    .page(self.artists_view.clone().into_any_element())
                                    .page(self.albums_view.clone().into_any_element())
                                    .page(div().into_any_element())
                                    .page(self.other_sources_view.clone().into_any_element()),
                            )
                            .child(play_queue()),
                    )
                    .child(self.transport_controls.clone().into_any_element()),
            )
            .application_menu(self.application_menu.clone())
    }
}
