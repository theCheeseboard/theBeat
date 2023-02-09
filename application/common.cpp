#include "common.h"

#include <QMenu>
#include <QStringList>
#include <burnbackend.h>
#include <burnmanager.h>
#include <statemanager.h>

QString Common::durationToString(quint64 ms, bool zeroIsInfinity) {
    if (zeroIsInfinity && ms == 0) return "∞";
    QStringList parts;

    qint64 seconds = ms / 1000 % 60;
    qint64 minutes = ms / 1000 / 60 % 60;
    qint64 hours = ms / 1000 / 60 / 60;

    if (hours > 0) parts.append(QString::number(hours));
    parts.append(QStringLiteral("%1").arg(minutes, 2, 10, QLatin1Char('0')));
    parts.append(QStringLiteral("%1").arg(seconds, 2, 10, QLatin1Char('0')));

    return parts.join(":");
}

void Common::showBurnMenu(QStringList files, QString title, QWidget* atButton) {
    QList<BurnBackend*> backends = StateManager::instance()->burn()->availableBackends();

    if (backends.count() == 1) {
        backends.first()->burn(files, title, atButton->window());
    } else {
        QMenu* menu = new QMenu();
        menu->addSection(tr("Select Device"));
        for (BurnBackend* backend : backends) {
            menu->addAction(backend->displayName(), [=] {
                backend->burn(files, title, atButton->window());
            });
        }
        QObject::connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
        menu->popup(atButton->mapToGlobal(QPoint(0, atButton->height())));
    }
}
