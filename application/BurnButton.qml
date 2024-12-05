import QtQuick
import QtQuick.Controls
import com.vicr123.Contemporary

Button {
    id: root

    property var model
    property string albumName

    text: qsTr("Burn")
    icon.name: "tools-media-optical-burn"
    visible: BurnManager.availableBackends.length > 0

    onClicked: () => {
        if (BurnManager.availableBackends.length > 1) {
            burnMenu.popup();
        } else {
            burn(BurnManager.availableBackends[0]);
        }
    }

    function burn(backend) {
        const paths = [];
        for (var i = 0; i < root.model.rowCount(); i++) {
            paths.push(root.model.path(i));
        }
        backend.burn(paths, root.albumName, Window.window);
    }

    Menu {
        id: burnMenu

        MenuSection {
            text: qsTr("Select Device")
        }

        Instantiator {
            id: recentFilesInstantiator
            model: BurnManager.availableBackends
            delegate: MenuItem {
                required property var modelData
                text: modelData.displayName
                onTriggered: () => {
                    burn(modelData);
                }

                Loader {
                    id: uiLoader
                }

                Component.onCompleted: () => {
                    uiLoader.setSource(modelData.qmlFile, {
                        controller: modelData
                    });
                }
            }

            onObjectAdded: (index, object) => burnMenu.insertItem(index + 1, object)
            onObjectRemoved: (index, object) => burnMenu.removeItem(object)
        }
    }
}
