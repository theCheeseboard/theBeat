import QtQuick
import Contemporary
import com.vicr123.Contemporary
import QtQuick.Controls

Item {
    id: root

    QtObject {
        id: d

        property var sources: []
    }

    function addSource(source) {
        if (source.qmlFile === "")
            return;
        console.log(`Add Source ${source.qmlFile}`);
        const component = Qt.createComponent(source.qmlFile, this);
        if (!component) {
            console.error(`Unable to add source with QML file ${source.qmlFile}`);
            console.error("Qt.createComponent returned null");
            return;
        }
        if (component.status === Component.Loading) {
            component.statusChanged.connect(() => {
                finaliseAddSource(source, component);
            });
        } else {
            finaliseAddSource(source, component);
        }
    }

    function finaliseAddSource(source, component) {
        if (component.status === Component.Ready) {
            const object = component.createObject(stack, {
                source: source
            });
            stack.push(object);
            const sidebarItem = sidebarDelegate.createObject(sidebarModel, {
                source: source
            });
            sidebarModel.append(sidebarItem);
            d.sources.push({
                source: source,
                sidebarItem: sidebarItem,
                component: object
            });
        } else {
            addSourceErrorPane(source, component.errorString);
        }
    }

    function addSourceErrorPane(source, error) {
        console.error(`Unable to add source with QML file ${source.qmlFile}`);
        console.error(error);
        stack.push(errorComponent);
        const sidebarItem = sidebarDelegate.createObject(sidebarModel, {
            source: source
        });
        sidebarModel.append(sidebarItem);
        d.sources.push({
            source: source,
            sidebarItem: sidebarItem,
            component: errorComponent
        });
    }

    function removeSource(source) {
        const sourceDef = d.sources.find(s => s.source === source);
        if (!sourceDef)
            return;
        for (let i = 0; i < sidebarModel.count; i++) {
            if (sidebarModel.get(i) === sourceDef.sidebarItem) {
                sidebarModel.remove(i, 1);
            }
        }
        stack.remove(sourceDef.component);
    }

    Component.onCompleted: () => {
        for (const source of SourceManager.sources()) {
            addSource(source);
        }
    }

    Connections {
        target: SourceManager
        function onSourceAdded(source) {
            root.addSource(source);
        }
        function onSourceRemoved(source) {
            root.removeSource(source);
        }
    }

    Component {
        id: errorComponent

        Interstitial {
            text: qsTr("Oh, bonkers!")
            subtitle: qsTr("This source can't be loaded right now")
        }
    }

    Component {
        id: sidebarDelegate

        Item {
            id: sidebarItem
            required property var source

            implicitHeight: childrenRect.height + 3
            implicitWidth: sidebar.width

            MouseArea {
                anchors.fill: parent

                onClicked: sidebar.currentIndex = sidebarItem.ObjectModel.index
            }
            Label {
                id: sidebarDelegateLabel

                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.right: parent.right
                anchors.rightMargin: 6
                anchors.top: parent.top
                anchors.topMargin: 3
                text: source.name
            }
        }
    }

    Rectangle {
        id: sidebarContainer

        LayerCalculator {
            id: layer1
            layer: 1
        }

        LayerCalculator {
            id: layer2
            layer: 2
        }

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        color: layer1.color
        radius: 4
        width: 300
        z: 10

        Grandstand {
            id: title

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: layer2.color
            innerTopMargin: SafeZone.top
            text: qsTr("Other Sources")
            z: 10
        }
        ListView {
            id: sidebar

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: title.bottom
            anchors.topMargin: 6
            displayMarginBeginning: SafeZone.top
            displayMarginEnd: SafeZone.bottom
            focus: true

            highlight: Item {
                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: 3
                    anchors.rightMargin: 3
                    color: Contemporary.accent
                    radius: 4
                }
            }
            model: ObjectModel {
                id: sidebarModel
            }

            onCurrentIndexChanged: stack.currentIndex = sidebar.currentIndex
        }
    }
    Pager {
        id: stack

        anchors.bottom: parent.bottom
        anchors.left: sidebarContainer.right
        anchors.leftMargin: 3
        anchors.right: parent.right
        anchors.top: parent.top
        currentAnimation: Pager.Animation.Lift

        defaultItem: Interstitial {
            anchors.fill: parent

            icon.name: "view-list-details"
            text: qsTr("No other sources available")
            subtitle: qsTr("There's nothing else to play right now")
        }
    }
}
