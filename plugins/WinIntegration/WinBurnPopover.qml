import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary

Drawer {
    id: popover
    property var controller
    width: window.width
    height: Math.max(window.height - 300, 300)
    edge: Qt.BottomEdge
    interactive: false
    visible: controller.visible

    Grandstand {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        backButtonVisible: true
        text: qsTr("Burn %1").arg(Contemporary.quoteString(controller.albumName))

        onBackButtonClicked: controller.close()
    }

    Pager {
        anchors.topMargin: 3
        anchors.top: grandstand.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: controller.imageReady ? 1 : 0

        Item {
            BusyIndicator {
                anchors.centerIn: parent
            }
        }

        ColumnLayout {
            GroupBox {
                title: qsTr("Burn Options")
                implicitWidth: 600

                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    rows: 2

                    Label {
                        Layout.row: 0
                        Layout.column: 0
                        text: qsTr("Album Name")
                    }

                    TextField {
                        id: albumNameField
                        Layout.fillWidth: true
                        Layout.row: 0
                        Layout.column: 1
                        placeholderText: qsTr("Album Name")
                        text: controller.albumName
                        onTextEdited: () => {
                            controller.albumName = albumNameField.text;
                        }
                    }
                }
            }

            Admonition {
                implicitWidth: 600
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                visible: !!controller.admonition
                title: qsTr("Heads up!")
                text: controller.admonition
                severity: controller.admonitionIsError ? Admonition.Severity.Error : Admonition.Severity.Warning
            }

            Button {
                implicitWidth: 600
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                text: qsTr("Burn")
                icon.name: "tools-media-optical-burn"
                enabled: !controller.admonitionIsError

                onClicked: () => controller.startBurn()
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
