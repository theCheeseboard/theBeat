#include "macburnprovider.h"

#include <statemanager.h>
#include <burnmanager.h>

MacBurnProvider::MacBurnProvider(QObject* parent) : BurnBackend(parent) {
    StateManager::instance()->burn()->registerBackend(this);
}

QString MacBurnProvider::displayName() {
    return tr("Burn on macOS");
}
