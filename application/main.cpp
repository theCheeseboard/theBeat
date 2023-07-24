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

#include "library/librarymanager.h"
#include "qtmultimedia/qtmultimediaurlhandler.h"
#include "settingspanes/colourssettingspane.h"
#include "settingspanes/libraryresetsettingspane.h"
#include "settingspanes/notificationssettingspane.h"
#include "settingspanes/titlebarsettingspane.h"
#include "thememanager.h"
#include <QCommandLineParser>
#include <QDir>
#include <QJsonArray>
#include <QUrl>
#include <dependencyinjection/tdimanager.h>
#include <dependencyinjection/tinjectedpointer.h>
#include <playlist.h>
#include <plugins/tpluginmanager.h>
#include <plugins/tpluginmanagerpane.h>
#include <statemanager.h>
#include <tapplication.h>
#include <thebeatcommon.h>
#include <thebeatplugininterface.h>
#include <tsettings.h>
#include <tsettingswindow/tsettingswindow.h>
#include <tstylemanager.h>
#include <urlmanager.h>

#include "visualisations/scopevisualisation.h"
#include <visualisationmanager.h>

#ifdef HAVE_THEINSTALLER
    #include <updatechecker.h>
#endif

int main(int argc, char* argv[]) {
    if (!qEnvironmentVariableIsSet("QT_MULTIMEDIA_PREFERRED_PLUGINS")) qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", "windowsmediafoundation");
    tApplication a(argc, argv);
    a.setApplicationShareDir("thebeat");
    a.installTranslators();

    QString dir = SYSTEM_PREFIX_DIRECTORY;

    a.setApplicationVersion("4.0.1");
    a.setGenericName(QApplication::translate("main", "Audio Player"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2023");
    a.setOrganizationName("theSuite");
    a.setApplicationUrl(tApplication::HelpContents, QUrl("https://help.vicr123.com/docs/thebeat/intro"));
    a.setApplicationUrl(tApplication::Sources, QUrl("http://github.com/vicr123/theBeat"));
    a.setApplicationUrl(tApplication::FileBug, QUrl("http://github.com/vicr123/theBeat/issues"));
    a.setApplicationName(T_APPMETA_READABLE_NAME);
    a.setDesktopFileName(T_APPMETA_DESKTOP_ID);

    a.registerCrashTrap();

#if defined(Q_OS_WIN)
    a.setWinApplicationClassId("{98fd3bc5-b39c-4c97-b483-4c95b90a7c39}");
#elif defined(Q_OS_MAC)
    a.setQuitOnLastWindowClosed(false);
#endif

    TheBeatCommon::addLibTheBeat(&a);

    tSettings settings;

    auto urlManager = a.dependencies()->tBaseDIManager::requiredService<IUrlManager>();

    urlManager->registerHandler(new QtMultimediaUrlHandler());

    StateManager::instance()->visualisation()->registerEngine("scope", new ScopeVisualisation());
    StateManager::instance()->visualisation()->setCurrentEngine("scope");

    auto pluginManager = tPluginManager<TheBeatPluginInterface>::instance();
    pluginManager->setLibraryDirectory("thebeat");

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

    QObject::connect(&settings, &tSettings::settingChanged, [=](QString key, QVariant value) {
        if (key == "theme/mode") {
            tStyleManager::setOverrideStyleForApplication(value.toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);
        }
    });
    tStyleManager::setOverrideStyleForApplication(settings.value("theme/mode").toString() == "light" ? tStyleManager::ContemporaryLight : tStyleManager::ContemporaryDark);

    auto w = a.dependencies()->construct<MainWindow>();

    QObject::connect(&a, &tApplication::singleInstanceMessage, [=](QJsonObject launchMessage) {
        if (launchMessage.contains("files")) {
            QJsonArray files = launchMessage.value("files").toArray();
            MediaItem* firstItem = nullptr;
            for (const QJsonValue& file : files) {
                MediaItem* item = urlManager->itemForUrl(QUrl(file.toString()));
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
    QObject::connect(&a, &tApplication::dockIconClicked, [=] {
        w->show();
        w->activateWindow();
    });
    QObject::connect(&a, &tApplication::openFile, [=](QString file) {
        MediaItem* item = urlManager->itemForUrl(QUrl::fromLocalFile(file));
        StateManager::instance()->playlist()->addItem(item);
        StateManager::instance()->playlist()->setCurrentItem(item);
        StateManager::instance()->playlist()->play();
    });

    QStringList files;
    for (const QString& arg : parser.positionalArguments()) {
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

    tSettingsWindow::addStaticSection(0, "general", a.translate("main", "General"));
    tSettingsWindow::addStaticPane(10, "general", [] {
        return new NotificationsSettingsPane();
    });
    tSettingsWindow::addStaticPane(100, "general", [] {
        return new tPluginManagerPane();
    });

    tSettingsWindow::addStaticSection(10, "appearance", a.translate("main", "Appearance"));
    tSettingsWindow::addStaticPane(10, "appearance", [] {
        return new TitlebarSettingsPane();
    });

    if (tStyleManager::isOverridingStyle()) {
        tSettingsWindow::addStaticPane(20, "appearance", [] {
            return new ColoursSettingsPane();
        });
    }

    tSettingsWindow::addStaticSection(20, "library", a.translate("main", "Library"));
    tSettingsWindow::addStaticPane(10, "library", [] {
        return new LibraryResetSettingsPane();
    });

    w->show();

    MediaItem* firstItem = nullptr;
    for (QString file : files) {
        MediaItem* item = urlManager->itemForUrl(QUrl(file));
        StateManager::instance()->playlist()->addItem(item);
        if (!firstItem) firstItem = item;
    }
    if (firstItem) {
        StateManager::instance()->playlist()->setCurrentItem(firstItem);
        StateManager::instance()->playlist()->play();
    }

    int retval = a.exec();

    StateManager::instance()->playlist()->pause();

    return retval;
}
