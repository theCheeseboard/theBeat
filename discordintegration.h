#ifndef DISCORDINTEGRATION_H
#define DISCORDINTEGRATION_H

#include <QObject>
#include <discord_rpc.h>
#include <QLibrary>
#include <QVariantMap>
#include <QTimer>

typedef void (*InitDiscordFunction)(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId);
typedef void (*UpdateDiscordFunction)(const DiscordRichPresence* presence);

class DiscordIntegration : public QObject
{
        Q_OBJECT
    public:
        explicit DiscordIntegration(QObject *parent = nullptr);

    signals:

    public slots:
        void updateState(QString interface, QVariantMap mprisDataMap, QStringList invalidatedProperties);

    private:
        bool working = false;
        QVariantMap currentState;

        InitDiscordFunction Discord_Initialize;
        UpdateDiscordFunction Discord_UpdatePresence;

        QTimer normalisationTimer;
};

#endif // DISCORDINTEGRATION_H
