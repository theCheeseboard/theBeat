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
#include <QLibrary>
#include <QDateTime>
#include <playlist.h>
#include <tapplication.h>
#include <discord_rpc.h>

typedef void (*InitDiscordFunction)(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId);
typedef void (*UpdateDiscordFunction)(const DiscordRichPresence* presence);
typedef void (*DiscordRespondFunction)(const char* userid, int reply);
typedef void (*DiscordRunCallbacksFunction)(void);

#define TO_CONST_CHAR(string, bufSize) ([=]() -> char* { \
        char* strBuf = new char[bufSize]; \
        sprintf(strBuf, "%s", string.toUtf8().constData()); \
        d->bufsToDelete.append(strBuf); \
        return strBuf; \
    })()

struct PluginPrivate {
    bool discordAvailable = false;
    MediaItem* currentItem = nullptr;

    InitDiscordFunction Discord_Initialize;
    UpdateDiscordFunction Discord_UpdatePresence;
    DiscordRespondFunction Discord_Respond;
    DiscordRunCallbacksFunction Discord_RunCallbacks;
};

Plugin::Plugin() {
    d = new PluginPrivate();

    tApplication::addPluginTranslator("drpintegration");

#ifdef DISCORD_STATIC
    d->Discord_Initialize = &::Discord_Initialize;
    d->Discord_UpdatePresence = &::Discord_UpdatePresence;
    d->Discord_Respond = &::Discord_Respond;
    d->Discord_RunCallbacks = &::Discord_RunCallbacks;
    d->discordAvailable = true;
#else
    QLibrary* lib;
#ifdef Q_OS_LINUX
    lib = new QLibrary("libdiscord-rpc.so");
#elif defined(Q_OS_WIN)
    lib = new QLibrary("discord-rpc.dll");
#endif

    if (lib->load()) {
        d->Discord_Initialize = reinterpret_cast<InitDiscordFunction>(lib->resolve("Discord_Initialize"));
        d->Discord_UpdatePresence = reinterpret_cast<UpdateDiscordFunction>(lib->resolve("Discord_UpdatePresence"));
        d->Discord_Respond = reinterpret_cast<DiscordRespondFunction>(lib->resolve("Discord_Respond"));
        d->Discord_RunCallbacks = reinterpret_cast<DiscordRunCallbacksFunction>(lib->resolve("Discord_RunCallbacks"));
        d->discordAvailable = true;
    }
#endif

}

Plugin::~Plugin() {
    delete d;
}

void Plugin::updateCurrentItem() {
    if (d->currentItem != StateManager::instance()->playlist()->currentItem() && d->currentItem != nullptr) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, &Plugin::updateItemMetadata);
        updateItemMetadata();
    } else {
        DiscordRichPresence presence;
        memset(&presence, 0, sizeof(presence));

        char details[128];
        sprintf(details, "%s", tr("Paused").toLocal8Bit().data());
        presence.details = details;
        presence.largeImageKey = "thebeat-logo";
        presence.largeImageText = "theBeat";

        Discord_UpdatePresence(&presence);

    }
}

void Plugin::updateItemMetadata() {
    //Construct a Rich Presence from the current state
    DiscordRichPresence presence;
    memset(&presence, 0, sizeof(presence));

    if (StateManager::instance()->playlist()->state() == Playlist::Playing) {
        QString detailsString = d->currentItem->title();
        detailsString.truncate(127);

        char details[128];
        sprintf(details, "%s", detailsString.toLocal8Bit().data());
        presence.details = details;

        QStringList stateString;
        if (!d->currentItem->authors().isEmpty()) {
            stateString.append(QLocale().createSeparatedList(d->currentItem->authors()));
        }
        if (!d->currentItem->album().isEmpty()) {
            stateString.append(d->currentItem->album());
        }

        QString stateFinalString = stateString.join(" - ");
        stateFinalString.truncate(127);

        char state[128];
        sprintf(state, "%s", stateFinalString.toLocal8Bit().data());
        presence.state = state;
    } else {
        char details[128];
        sprintf(details, "%s", tr("Paused").toLocal8Bit().data());
        presence.details = details;
    }

    presence.largeImageKey = "thebeat-logo";
    presence.largeImageText = "theBeat";

    Discord_UpdatePresence(&presence);
}

void Plugin::activate() {
    if (d->discordAvailable) {
        DiscordEventHandlers handlers;
        memset(&handlers, 0, sizeof(handlers));
        handlers.ready = [](const DiscordUser * user) {
            qDebug() << "Discord Ready!";
        };
        handlers.errored = [](int errorCode, const char* message) {
            qDebug() << "Discord Error!";
            qDebug() << errorCode;
            qDebug() << message;
        };
        handlers.disconnected = [](int errorCode, const char* message) {
            qDebug() << "Discord Disconnected!";
        };
        Discord_Initialize("492669299446644748", &handlers, true, nullptr);
    }

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &Plugin::updateCurrentItem);
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, &Plugin::updateItemMetadata);
    updateCurrentItem();
}

void Plugin::deactivate() {

}
