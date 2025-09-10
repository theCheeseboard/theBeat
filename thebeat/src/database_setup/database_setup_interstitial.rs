use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::icon_text::icon_text;
use contemporary::components::interstitial::interstitial;
use gpui::{App, IntoElement, ParentElement, RenderOnce, Styled, Window, div};

#[derive(IntoElement)]
pub struct DatabaseSetupInterstitial {}

pub fn database_setup_interstitial() -> DatabaseSetupInterstitial {
    DatabaseSetupInterstitial {}
}

impl RenderOnce for DatabaseSetupInterstitial {
    fn render(self, _: &mut Window, _: &mut App) -> impl IntoElement {
        interstitial()
            .icon("view-media-track".into())
            .title(tr!("DATABASE_SETUP_INTERSTITIAL_TITLE", "Welcome to theBeat!").into())
            .message(
                tr!(
                    "DATABASE_SETUP_INTERSTITIAL_MESSAGE",
                    "Set up your library to see your music organised here"
                )
                .into(),
            )
            .child(
                button("database-setup-library-setup-button").child(icon_text(
                    "arrow-right".into(),
                    tr!("DATABASE_SETUP_INTERSTITIAL_BUTTON_TEXT", "Set up library").into(),
                )),
            )
            .size_full()
    }
}
