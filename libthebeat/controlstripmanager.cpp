#include "controlstripmanager.h"

ControlStripManager::ControlStripManager(QObject* parent) : QObject(parent) {

}

void ControlStripManager::addButton(QWidget* button) {
    emit buttonAdded(button);
}
