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
#include "plugin.h"

#include "podcastmanager.h"
#include "settingspanes/podcastsettingspane.h"
#include "widgets/podcastpane.h"
#include <QTimer>
#include <tapplication.h>
#include <tsettings.h>
#include <tsettingswindow/tsettingswindow.h>

struct PluginPrivate {
        PodcastPane* podcastPane;
        QTimer* podcastUpdateTimer;
};

inline void initResources() {
    Q_INIT_RESOURCE(PODCASTPLUGIN);
}

Plugin::Plugin() {
    d = new PluginPrivate();

    tApplication::addPluginTranslator(CNTP_SHARE_DIR);
}

Plugin::~Plugin() {
    delete d;
}

void Plugin::activate() {
    initResources();

    tSettings::registerDefaults(":/plugins/podcast/defaults.conf");

    PodcastManager::instance()->init();
    PodcastManager::instance()->updatePodcasts(true);

    d->podcastPane = new PodcastPane();

    // Set up timer to update podcasts periodically
    d->podcastUpdateTimer = new QTimer();
    d->podcastUpdateTimer->setInterval(15 * 60 * 1000);
    connect(d->podcastUpdateTimer, &QTimer::timeout, this, [] {
        PodcastManager::instance()->updatePodcasts(true);
    });
    d->podcastUpdateTimer->start();

    tSettingsWindow::addStaticPane(20, "general", [] {
        return new PodcastSettingsPane();
    });
}

void Plugin::deactivate() {
}
