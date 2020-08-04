/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "mainwindow.h"

#include <QDir>
#include <QUrl>
#include <tapplication.h>
#include <tsettings.h>

#ifdef HAVE_THEINSTALLER
    #include <updatechecker.h>
#endif

int main(int argc, char* argv[]) {
    if (!qEnvironmentVariableIsSet("QT_MULTIMEDIA_PREFERRED_PLUGINS")) qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", "windowsmediafoundation");
    tApplication a(argc, argv);

    if (QDir("/usr/share/thebeat/").exists()) {
        a.setShareDir("/usr/share/thebeat/");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/thebeat/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/thebeat/"));
    }
    a.installTranslators();

    a.setApplicationIcon(QIcon::fromTheme("thebeat", QIcon(":/icons/thebeat.svg")));
    a.setApplicationVersion("3.0");
    a.setGenericName(QApplication::translate("main", "Audio Player"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2020");
    a.setOrganizationName("theSuite");
#ifdef T_BLUEPRINT_BUILD
    a.setApplicationName("theBeat Blueprint");
    a.setDesktopFileName("com.vicr123.thebeat-blueprint");
#else
    a.setApplicationName("theBeat");
    a.setDesktopFileName("com.vicr123.thebeat");
#endif

    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
    tSettings::registerDefaults("/etc/theSuite/theBeat/defaults.conf");

#if defined(Q_OS_WIN)
    //Set up the theming
    a.setStyle(QStyleFactory::create("contemporary"));

    QIcon::setThemeName("contemporary-icons");
    QIcon::setThemeSearchPaths({a.applicationDirPath() + "\\icons"});

    //Get the accent colour
    QSettings accentDetection("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM", QSettings::NativeFormat);
    QColor accentCol(QRgb(accentDetection.value("ColorizationColor", 0xC4003296).toInt() & 0x00FFFFFF));

    QPalette pal = a.palette();

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

    a.setPalette(pal);
    a.setPalette(pal, "QDockWidget");
    a.setPalette(pal, "QToolBar");

    a.setWinApplicationClassId("{98fd3bc5-b39c-4c97-b483-4c95b90a7c39}");
#endif

#ifdef HAVE_THEINSTALLER
    UpdateChecker::initialise(QUrl("https://vicr123.com/thebeat/theinstaller/installer.json"), QUrl("https://github.com/vicr123/theBeat/releases"), 3, 0, 0, 0);
    QObject::connect(UpdateChecker::instance(), &UpdateChecker::closeAllWindows, &a, &tApplication::quit);
#endif

    MainWindow w;
    w.show();
    return a.exec();
}
