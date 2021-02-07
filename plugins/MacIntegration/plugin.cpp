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

#include <QDebug>
#include <statemanager.h>
#include <sourcemanager.h>
#include <pluginmediasource.h>
#include <QIcon>
#include <tapplication.h>
#include "mainwindowtouchbar.h"
#include "cd/diskwatcher.h"
#include "burn/macburnprovider.h"
#include "nowplayingintegration.h"

struct PluginPrivate {

};

Plugin::Plugin() {
    d = new PluginPrivate();
    tApplication::addPluginTranslator("macintegration");
}

Plugin::~Plugin() {
    delete d;
}


void Plugin::activate() {
    new MainWindowTouchBar(StateManager::instance()->mainWindow());
    new DiskWatcher();
    new MacBurnProvider();
    new NowPlayingIntegration();
}

void Plugin::deactivate() {

}
