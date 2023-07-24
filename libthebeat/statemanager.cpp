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
#include "statemanager.h"

#include "burnmanager.h"
#include "controlstripmanager.h"
#include "playlist.h"
#include "sourcemanager.h"
#include "urlmanager.h"
#include "visualisationmanager.h"

struct StateManagerPrivate {
        Playlist* playlist;
        SourceManager* sources;
        BurnManager* burn;
        VisualisationManager* visualisation;
        ControlStripManager* controlStrip;
//        UrlManager* url;

        QWidget* mainWindow = nullptr;
};

StateManager::StateManager(QObject* parent) :
    QObject(parent) {
    d = new StateManagerPrivate();
    d->playlist = new Playlist();
    d->sources = new SourceManager();
    d->burn = new BurnManager();
    d->visualisation = new VisualisationManager();
    d->controlStrip = new ControlStripManager();
//    d->url = new UrlManager();
}

StateManager* StateManager::instance() {
    static StateManager* mgr = new StateManager();
    return mgr;
}

Playlist* StateManager::playlist() {
    return d->playlist;
}

SourceManager* StateManager::sources() {
    return d->sources;
}

BurnManager* StateManager::burn() {
    return d->burn;
}

VisualisationManager* StateManager::visualisation() {
    return d->visualisation;
}

// IUrlManager* StateManager::url() {
//     return d->url;
// }

ControlStripManager* StateManager::controlStrip() {
    return d->controlStrip;
}

QWidget* StateManager::mainWindow() {
    return d->mainWindow;
}

void StateManager::setMainWindow(QWidget* mainWindow) {
    d->mainWindow = mainWindow;
    emit mainWindowAvailable(mainWindow);
}
