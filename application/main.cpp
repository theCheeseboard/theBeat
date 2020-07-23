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
#include <tapplication.h>
#include <tsettings.h>

int main(int argc, char* argv[]) {
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

    MainWindow w;
    w.show();
    return a.exec();
}
