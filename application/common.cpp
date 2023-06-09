#include "common.h"

#include <QMenu>
#include <QStringList>
#include <burnbackend.h>
#include <burnmanager.h>
#include <statemanager.h>

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
