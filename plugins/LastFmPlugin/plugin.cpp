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

#include "lastfmapiservice.h"
#include "scrobbleservice.h"
#include "settings/lastfmsettingspane.h"
#include <QDebug>
#include <QIcon>
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>
#include <tapplication.h>
#include <tsettingswindow/tsettingswindow.h>
#include <urlmanager.h>

struct PluginPrivate {
        ScrobbleService* scrobbleService;
};

Plugin::Plugin() {
    d = new PluginPrivate();
    tApplication::addPluginTranslator(CNTP_SHARE_DIR);
}

Plugin::~Plugin() {
    delete d;
}

void Plugin::activate() {
    tSettingsWindow::addStaticSection(15, "lastfm", tr("last.fm"));
    tSettingsWindow::addStaticPane(10, "lastfm", [] {
        return new LastFmSettingsPane();
    });

    d->scrobbleService = new ScrobbleService();

    // Attempt to scrobble any remaining pending scrobbles
    LastFmApiService::scrobble();
}

void Plugin::deactivate() {
    delete d->scrobbleService;
}
