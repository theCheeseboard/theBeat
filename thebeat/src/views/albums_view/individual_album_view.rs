use crate::track_listing::track_listing;
use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::Theme;
use gpui::private::anyhow;
use gpui::{div, linear_color_stop, linear_gradient, px, quad, rgb, transparent_black, uniform_list, Along, Anchor, AnyElement, App, AppContext, AsyncApp, Axis, Background, BorderStyle, Bounds, Context, Corners, Edges, EdgesRefinement, Element, ElementId, Entity, GlobalElementId, InspectorElementId, IntoElement, LayoutId, LinearColorStop, ParentElement, Pixels, Refineable, Render, RenderImage, Rgba, Size, Style, StyleRefinement, Styled, Subscription, WeakEntity, Window};
use lthebeat::audio_library::album::Album;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::track::Track;
use lthebeat::audio_processing::audio_metadata::Art;
use std::cell::RefCell;
use std::ops::Deref;
use std::panic::Location;
use std::rc::Rc;
use std::sync::Arc;

pub struct IndividualAlbumView {
    album_name: String,
    database_subscription: Subscription,
    tracks_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>>,
    on_back_clicked: Rc<Box<dyn Fn(&(), &mut Window, &mut App) + 'static>>,
    album_art: Option<Art>,
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
                album_art,
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

        album_background(
            self.album_art.clone(),
            self.album_art
                .as_ref()
                .and_then(|art| art.average_color())
                .unwrap_or(theme.background)
                .blend(Rgba {
                    a: 0.9,
                    ..theme.background
                }),
        )
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
                    .flex_grow(1.)
                    .w_full()
                    .child(track_listing(tracks_query.clone()))
                    .into_any_element(),
                Err(_) => interstitial()
                    .w_full()
                    .h_full()
                    .icon("media-album-cover")
                    .title(tr!("LIBRARY_ALBUM_ERROR", "Unable to load album"))
                    .message(tr!("LIBRARY_CORRUPT_ERROR_MESSAGE"))
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

struct AlbumBackground {
    style: StyleRefinement,
    children: Vec<AnyElement>,
    art: Option<Art>,
    bg: Rgba,
}

fn album_background(art: Option<Art>, bg: Rgba) -> AlbumBackground {
    AlbumBackground {
        style: StyleRefinement::default(),
        children: Vec::new(),
        art,
        bg,
    }
}

impl IntoElement for AlbumBackground {
    type Element = Self;

    fn into_element(self) -> Self::Element {
        self
    }
}

struct AlbumBackgroundPrepaintState {
    background_art: Option<(Arc<RenderImage>, Bounds<Pixels>)>,
    background: Background,
    cutoff_bounds: Bounds<Pixels>,
    shade_bounds: Bounds<Pixels>,
    shade: Background,
}

impl Element for AlbumBackground {
    type RequestLayoutState = ();
    type PrepaintState = AlbumBackgroundPrepaintState;

    fn id(&self) -> Option<ElementId> {
        Some("individual-album-background".into())
    }

    fn source_location(&self) -> Option<&'static Location<'static>> {
        None
    }

    fn request_layout(
        &mut self,
        id: Option<&GlobalElementId>,
        inspector_id: Option<&InspectorElementId>,
        window: &mut Window,
        cx: &mut App,
    ) -> (LayoutId, Self::RequestLayoutState) {
        let children: Vec<_> = self
            .children
            .iter_mut()
            .map(|element| element.request_layout(window, cx))
            .collect();

        let layout_id =
            window.request_layout(Style::default().refined(self.style.clone()), children, cx);

        (layout_id, ())
    }

    fn prepaint(
        &mut self,
        id: Option<&GlobalElementId>,
        inspector_id: Option<&InspectorElementId>,
        bounds: Bounds<Pixels>,
        request_layout: &mut Self::RequestLayoutState,
        window: &mut Window,
        cx: &mut App,
    ) -> Self::PrepaintState {
        const HEADER_SIZE: f32 = 150.;

        for element in self.children.iter_mut() {
            element.prepaint(window, cx);
        }

        let transparent_background = Rgba { a: 0.6, ..self.bg };

        let mut prepaint_state = AlbumBackgroundPrepaintState {
            background_art: None,
            background: self.bg.into(),
            cutoff_bounds: Bounds::from_corners(
                bounds
                    .origin
                    .apply_along(Axis::Vertical, |y| y + px(HEADER_SIZE)),
                bounds.bottom_right(),
            ),
            shade_bounds: Bounds::from_anchor_and_size(
                Anchor::TopLeft,
                bounds.origin,
                bounds.size.apply_along(Axis::Vertical, |_| px(HEADER_SIZE)),
            ),
            shade: linear_gradient(
                0.,
                linear_color_stop(self.bg, 0.),
                linear_color_stop(transparent_background, 1.),
            ),
        };

        if let Some(art) = &self.art {
            if let Some(render_image) = art.render_image() {
                let dimensions = art
                    .dimensions()
                    .expect("Loaded image should have dimensions");

                let ratio = dimensions.0 as f32 / dimensions.1 as f32;

                prepaint_state.background_art = Some((
                    render_image,
                    Bounds::from_anchor_and_size(
                        Anchor::TopLeft,
                        bounds.origin.apply_along(Axis::Vertical, |y| {
                            y - bounds.size.width / ratio / 2. + px(HEADER_SIZE / 2.)
                        }),
                        Size::new(bounds.size.width, bounds.size.width / ratio),
                    ),
                ));
            }
        }

        prepaint_state
    }

    fn paint(
        &mut self,
        id: Option<&GlobalElementId>,
        inspector_id: Option<&InspectorElementId>,
        bounds: Bounds<Pixels>,
        request_layout: &mut Self::RequestLayoutState,
        prepaint: &mut Self::PrepaintState,
        window: &mut Window,
        cx: &mut App,
    ) {
        window.paint_quad(quad(
            bounds,
            Corners::all(px(0.)),
            prepaint.background,
            px(0.),
            transparent_black(),
            BorderStyle::Solid,
        ));

        if let Some((render_image, bounds)) = prepaint.background_art.take() {
            window
                .paint_image(bounds, Corners::all(px(0.)), render_image, 0, false)
                .unwrap();

            window.paint_quad(quad(
                prepaint.shade_bounds,
                Corners::all(px(0.)),
                prepaint.shade,
                px(0.),
                transparent_black(),
                BorderStyle::Solid,
            ));

            window.paint_quad(quad(
                prepaint.cutoff_bounds,
                Corners::all(px(0.)),
                prepaint.background,
                px(0.),
                transparent_black(),
                BorderStyle::Solid,
            ));
        }

        for element in self.children.iter_mut() {
            element.paint(window, cx);
        }
    }
}

impl Styled for AlbumBackground {
    fn style(&mut self) -> &mut StyleRefinement {
        &mut self.style
    }
}

impl ParentElement for AlbumBackground {
    fn extend(&mut self, elements: impl IntoIterator<Item = AnyElement>) {
        self.children.extend(elements);
    }
}
