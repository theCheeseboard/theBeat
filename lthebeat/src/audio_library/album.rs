use crate::audio_library::database_query::DatabaseRecord;
use crate::audio_processing::audio_metadata::Art;
use gpui::prelude::FluentBuilder;
use gpui::{
    Context, ImageSource, IntoElement, ParentElement, Render, Styled, Window, div, img, px, rgb,
    rgba,
};
use sqlx::sqlite::SqliteRow;
use sqlx::{Error, Row};

#[derive(Default, Clone)]
pub enum Album {
    Ok {
        id: u32,
        name: Option<String>,
        art: Option<Art>,
    },

    #[default]
    Loading,
    Error,
}

impl Render for Album {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        match self {
            Album::Ok { name, art, .. } => {
                let art = art
                    .clone()
                    .and_then(|album_cover| album_cover.render_image())
                    .clone();

                div()
                    .flex()
                    .flex_col()
                    .w(px(192.))
                    .child(
                        div()
                            .size(px(192.))
                            .when_some(art.clone(), |div, album_cover| {
                                div.child(img(ImageSource::Render(album_cover)).h_full().w_full())
                            })
                            .when_none(&art, |div| div.bg(rgb(0xFF0000))),
                    )
                    .child(name.clone().unwrap_or("Album".to_string()))
            }
            Album::Loading => div(),
            Album::Error => div(),
        }
    }
}

impl DatabaseRecord for Album {
    fn read_from_row(&mut self, row: Result<SqliteRow, Error>) {
        if let Ok(row) = row {
            *self = Album::Ok {
                id: row.get::<u32, _>("id"),
                name: row.get::<Option<String>, _>("name"),
                art: {
                    let image = row.get::<Option<Box<[u8]>>, _>("image");
                    let image_mime_type = row.get::<Option<String>, _>("image_mime_type");

                    if let Some(image) = image
                        && let Some(image_mime_type) = image_mime_type
                    {
                        Some(Art::new(image, image_mime_type))
                    } else {
                        None
                    }
                },
            }
        } else {
            *self = Album::Error;
        }
    }
}
