#include "macburnprovider.h"

#include <burnmanager.h>
#include <statemanager.h>

MacBurnProvider::MacBurnProvider(QObject* parent) :
    BurnBackend(parent) {
    StateManager::instance()->burn()->registerBackend(this);
}

QString MacBurnProvider::displayName() {
    return tr("Burn on macOS");
}
