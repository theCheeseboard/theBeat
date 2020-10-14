#include "thememanager.h"

#include <tsettings.h>

struct ThemeManagerPrivate {
    tSettings settings;
};

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    d = new ThemeManagerPrivate();

    connect(&d->settings, &tSettings::settingChanged, this, [=](QString key, QVariant value) {
        if (key == "theme/mode") updatePalette();
    });

    updatePalette();
}

void ThemeManager::updatePalette()
{
    //Get the accent colour
    QSettings accentDetection("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM", QSettings::NativeFormat);
    QColor accentCol(QRgb(accentDetection.value("ColorizationColor", 0xC4003296).toInt() & 0x00FFFFFF));

    QPalette pal = QApplication::palette();

    if (d->settings.value("theme/mode").toString() == "light") {
        pal.setColor(QPalette::Button, accentCol.lighter(150));
        pal.setColor(QPalette::ButtonText, QColor(0, 0, 0));
        pal.setColor(QPalette::Highlight, accentCol.lighter(125));
        pal.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
        pal.setColor(QPalette::Disabled, QPalette::Button, accentCol.darker(200));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0, 0, 0));

        pal.setColor(QPalette::Window, QColor(210, 210, 210));
        pal.setColor(QPalette::Base, QColor(210, 210, 210));
        pal.setColor(QPalette::AlternateBase, QColor(210, 210, 210));
        pal.setColor(QPalette::WindowText, QColor(0, 0, 0));
        pal.setColor(QPalette::Text, QColor(0, 0, 0));
        pal.setColor(QPalette::ToolTipText, QColor(0, 0, 0));

        pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(100, 100, 100));
    } else {
        pal.setColor(QPalette::Button, accentCol);
        pal.setColor(QPalette::ButtonText, QColor(255, 255, 255));
        pal.setColor(QPalette::Highlight, accentCol.lighter(125));
        pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        pal.setColor(QPalette::Disabled, QPalette::Button, accentCol.darker(200));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));

        pal.setColor(QPalette::Window, QColor(40, 40, 40));
        pal.setColor(QPalette::Base, QColor(40, 40, 40));
        pal.setColor(QPalette::AlternateBase, QColor(60, 60, 60));
        pal.setColor(QPalette::WindowText, QColor(255, 255, 255));
        pal.setColor(QPalette::Text, QColor(255, 255, 255));
        pal.setColor(QPalette::ToolTipText, QColor(255, 255, 255));

        pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(150, 150, 150));
    }

    QApplication::setPalette(pal);
    QApplication::setPalette(pal, "QDockWidget");
    QApplication::setPalette(pal, "QToolBar");
}
