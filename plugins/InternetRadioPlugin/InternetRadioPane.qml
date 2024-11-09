import QtQuick 2.15
import QtQuick.Controls
import com.vicr123.Contemporary
import Contemporary

Item {
    property var source

    Pager {
        anchors.fill: parent

        Item {
            Grandstand {
                id: title

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                innerTopMargin: SafeZone.top
                text: qsTr("Internet Radio")
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
            }
        }
    }
}
