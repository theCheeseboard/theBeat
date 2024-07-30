import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Contemporary

Rectangle {
    id: root

    property real innerTopMargin
    property alias text: label.text
    property bool backButtonVisible: false
    default property alias content: buttons.data
    signal backButtonClicked

    color: Contemporary.layer
    implicitHeight: Math.max(backButton.implicitHeight, label.implicitHeight) + innerTopMargin + 12 + buttons.childrenRect.height

    Layout.preferredHeight: implicitHeight

    radius: 4

    Item {
        anchors.fill: parent
        anchors.topMargin: innerTopMargin

        Button {
            id: backButton
            icon.name: "go-previous"
            anchors.left: parent.left
            anchors.top: parent.top
            width: backButtonVisible ? 32 : 0
            flat: true

            onClicked: root.backButtonClicked()
        }

        Label {
            id: label
            anchors.left: backButton.right
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: backButton.bottom
            anchors.margins: 6
            verticalAlignment: Qt.AlignVCenter
            font.pointSize: 15
        }

        Item {
            id: buttons
            anchors.top: backButton.bottom
            anchors.left: parent.left
            anchors.leftMargin: 9
            anchors.right: parent.right
            anchors.rightMargin: 9
            height: childrenRect.height
        }
    }
}
