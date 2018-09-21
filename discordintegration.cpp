#include "discordintegration.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusArgument>

DiscordIntegration::DiscordIntegration(QObject *parent) : QObject(parent)
{
    QLibrary* lib = new QLibrary("libdiscord-rpc.so");
    if (lib->load()) {
        Discord_Initialize = (InitDiscordFunction) lib->resolve("Discord_Initialize");
        Discord_UpdatePresence = (UpdateDiscordFunction) lib->resolve("Discord_UpdatePresence");

        DiscordEventHandlers handlers;
        memset(&handlers, 0, sizeof(handlers));
        handlers.ready = [](const DiscordUser* user) {
            qDebug() << "Discord Ready!";
        };
        handlers.errored = [](int errorCode, const char* message) {
            qDebug() << "Discord Error!";
        };
        handlers.disconnected = [](int errorCode, const char* message) {
            qDebug() << "Discord Disconnected!";
        };
        Discord_Initialize("492669299446644748", &handlers, true, nullptr);


        DiscordRichPresence presence;
        memset(&presence, 0, sizeof(presence));
        presence.state = "Idle";
        presence.largeImageKey = "thebeat-logo";
        presence.largeImageText = "theBeat";
        presence.instance = 1;
        Discord_UpdatePresence(&presence);

        QDBusConnection::sessionBus().connect("org.mpris.MediaPlayer2.theBeat", "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateState(QString,QVariantMap,QStringList)));

        normalisationTimer.setInterval(500);
        normalisationTimer.setSingleShot(true);
        connect(&normalisationTimer, &QTimer::timeout, [=] {
            //Construct a Rich Presence from the current state
            QVariantMap metadata = currentState.value("Metadata").value<QVariantMap>();

            DiscordRichPresence presence;
            memset(&presence, 0, sizeof(presence));

            QString detailsString;
            if (currentState.value("PlaybackStatus").toString() == "Playing") {
                detailsString = "Listening to %1";
                detailsString = detailsString.arg(metadata.value("xesam:title").toString());
            } else {
                detailsString = "Paused";
            }
            detailsString.truncate(127);

            char details[128];
            sprintf(details, "%s", detailsString.toLocal8Bit().data());
            presence.details = details;

            QStringList stateString;
            if (metadata.contains("xesam:artist")) {
                stateString.append(metadata.value("xesam:artist").toString());
            }
            if (metadata.contains("xesam:album")) {
                stateString.append(metadata.value("xesam:album").toString());
            }

            QString stateFinalString = stateString.join(" - ");
            stateFinalString.truncate(127);

            char state[128];
            sprintf(state, "%s", stateFinalString.toLocal8Bit().data());
            presence.state = state;

            presence.largeImageKey = "thebeat-logo";
            presence.largeImageText = "theBeat";

            Discord_UpdatePresence(&presence);
        });

        working = true;
    }
}

void DiscordIntegration::updateState(QString interface, QVariantMap mprisDataMap, QStringList invalidatedProperties) {
    if (working) {
        for (QString key : mprisDataMap.keys()) {
            if (key == "Metadata") {
                QDBusArgument arg = mprisDataMap.value(key).value<QDBusArgument>();
                QVariantMap demarshall;
                arg >> demarshall;
                currentState.insert(key, demarshall);
            } else {
                currentState.insert(key, mprisDataMap.value(key));
            }
        }

        //Give it a little to update
        if (normalisationTimer.isActive()) normalisationTimer.stop();
        normalisationTimer.start();
    }
}
