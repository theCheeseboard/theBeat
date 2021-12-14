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
#include "library/librarymanager.h"
#include <tapplication.h>
#include <tsettings.h>
#include <QCommandLineParser>
#include <tstylemanager.h>
#include <QJsonArray>
#include <statemanager.h>
#include <playlist.h>
#include "thememanager.h"
#include "qtmultimedia/qtmultimediaurlhandler.h"
#include <urlmanager.h>

#include <visualisationmanager.h>
#include "visualisations/scopevisualisation.h"

#ifdef HAVE_THEINSTALLER
    #include <updatechecker.h>
#endif

int main(int argc, char* argv[]) {
    if (!qEnvironmentVariableIsSet("QT_MULTIMEDIA_PREFERRED_PLUGINS")) qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", "windowsmediafoundation");
    tApplication a(argc, argv);

    QString dir = SYSTEM_PREFIX_DIRECTORY;

    if (QDir(QStringLiteral("%1/share/thebeat/").arg(SYSTEM_PREFIX_DIRECTORY)).exists()) {
        a.setShareDir(QStringLiteral("%1/share/thebeat/").arg(SYSTEM_PREFIX_DIRECTORY));
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/thebeat/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/thebeat/"));
    }
    a.installTranslators();

    a.setApplicationVersion("3.1.1");
    a.setGenericName(QApplication::translate("main", "Audio Player"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2021");
    a.setOrganizationName("theSuite");
    a.setApplicationUrl(tApplication::HelpContents, QUrl("https://help.vicr123.com/docs/thebeat/intro"));
    a.setApplicationUrl(tApplication::Sources, QUrl("http://github.com/vicr123/theBeat"));
    a.setApplicationUrl(tApplication::FileBug, QUrl("http://github.com/vicr123/theBeat/issues"));
#ifdef T_BLUEPRINT_BUILD
    a.setApplicationIcon(QIcon(":/icons/com.vicr123.thebeat_blueprint.svg"));
    a.setApplicationName("theBeat Blueprint");
    a.setDesktopFileName("com.vicr123.thebeat_blueprint");
#else
    a.setApplicationIcon(QIcon::fromTheme("thebeat", QIcon(":/icons/com.vicr123.thebeat.svg")));
    a.setApplicationName("theBeat");
    a.setDesktopFileName("com.vicr123.thebeat");
#endif

    a.registerCrashTrap();

#if defined(Q_OS_WIN)
    a.setWinApplicationClassId("{98fd3bc5-b39c-4c97-b483-4c95b90a7c39}");
    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
#elif defined(Q_OS_MAC)
    tSettings::registerDefaults(a.macOSBundlePath() + "/Contents/Resources/defaults.conf");
    a.setQuitOnLastWindowClosed(false);
#else
    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
    tSettings::registerDefaults("/etc/theSuite/theBeat/defaults.conf");
#endif

    tSettings settings;

    StateManager::instance()->url()->registerHandler(new QtMultimediaUrlHandler());

    StateManager::instance()->visualisation()->registerEngine("scope", new ScopeVisualisation());
    StateManager::instance()->visualisation()->setCurrentEngine("scope");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
//    parser.addOption({"force-contemporary-palette", a.translate("main", "Force theBeat to use the Contemporary colour palette")});
    parser.addPositionalArgument(a.translate("main", "file"), a.translate("main", "File to open"), QStringLiteral("[%1]").arg(a.translate("main", "file")));
    parser.process(a);

//    if (parser.isSet("force-contemporary-palette")) {
//        shouldLoadThemeManager = true;
//    }

//    if (shouldLoadThemeManager) {
//        new ThemeManager();
//    }

    QObject::connect(&settings, &tSettings::settingChanged, [ = ](QString key, QVariant value) {
        if (key == "theme/mode") {
            tStyleManager::setOverrideStyleForApplication(value.toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);
        }
    });
    tStyleManager::setOverrideStyleForApplication(settings.value("theme/mode").toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);

    MainWindow* w = new MainWindow();

    QObject::connect(&a, &tApplication::singleInstanceMessage, [ = ](QJsonObject launchMessage) {
        if (launchMessage.contains("files")) {
            QJsonArray files = launchMessage.value("files").toArray();
            MediaItem* firstItem = nullptr;
            for (QJsonValue file : files) {
                MediaItem* item = StateManager::instance()->url()->itemForUrl(QUrl(file.toString()));
                StateManager::instance()->playlist()->addItem(item);
                if (!firstItem) firstItem = item;
            }
            if (firstItem) {
                StateManager::instance()->playlist()->setCurrentItem(firstItem);
                StateManager::instance()->playlist()->play();
            } else {
                w->show();
                w->activateWindow();
            }
        }
    });
    QObject::connect(&a, &tApplication::dockIconClicked, [ = ] {
        w->show();
        w->activateWindow();
    });

    QStringList files;
    for (QString arg : parser.positionalArguments()) {
        if (QUrl::fromLocalFile(arg).isValid()) {
            files.append(QUrl::fromLocalFile(arg).toEncoded());
        } else {
            files.append(QUrl(arg).toEncoded());
        }
    }
    a.ensureSingleInstance({
        {"files", QJsonArray::fromStringList(files)}
    });

#ifdef HAVE_THEINSTALLER
    if (a.currentPlatform() != tApplication::WindowsAppPackage) {
        UpdateChecker::initialise(QUrl("https://vicr123.com/thebeat/theinstaller/installer.json"), QUrl("https://github.com/vicr123/theBeat/releases"), 3, 1, 1, 0);
        QObject::connect(UpdateChecker::instance(), &UpdateChecker::closeAllWindows, &a, &tApplication::quit);
    }
#endif

    w->show();

    MediaItem* firstItem = nullptr;
    for (QString file : files) {
        MediaItem* item = StateManager::instance()->url()->itemForUrl(QUrl(file));
        StateManager::instance()->playlist()->addItem(item);
        if (!firstItem) firstItem = item;
    }
    if (firstItem) {
        StateManager::instance()->playlist()->setCurrentItem(firstItem);
        StateManager::instance()->playlist()->play();
    }

    int retval = a.exec();

    return retval;
}
