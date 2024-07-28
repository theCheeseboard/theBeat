import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Contemporary

ListView {
    id: root
    spacing: 3

    delegate: Item {
        id: trackItem
        implicitWidth: containerRect.implicitWidth
        implicitHeight: containerRect.implicitHeight

        required property string path
        required property string title
        required property string artist
        required property string album
        required property int duration
        required property int track
        // required property var albumArt
        required property int error

        Rectangle {
            id: containerRect
            radius: 4
            color: mouseArea.containsMouse ? Contemporary.layer : Qt.rgba(0, 0, 0, 0)

            implicitHeight: itemLayout.implicitHeight + 6
            implicitWidth: root.width - 6
            x: 3

            GridLayout {
                id: itemLayout
                x: 3
                y: 3
                width: parent.width - 6
                rows: 2
                columns: 2
                columnSpacing: 3
                rowSpacing: 3

                Label {
                    Layout.row: 0
                    Layout.column: 0
                    Layout.rowSpan: 2

                    Layout.fillHeight: true
                    Layout.preferredWidth: implicitHeight

                    id: trackNumberLabel
                    text: trackItem.track === 0 ? "-" : trackItem.track
                    color: Contemporary.disabled(Contemporary.foreground)
                    verticalAlignment: Qt.AlignVCenter
                    horizontalAlignment: Qt.AlignHCenter
                    font.pointSize: trackNameLabel.font.pointSize * 2
                }

                RowLayout {
                    Layout.row: 0
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Label {
                        id: trackNameLabel
                        text: trackItem.title
                    }

                    Label {
                        id: trackDurationLabel
                        text: trackItem.duration
                        color: Contemporary.disabled(Contemporary.foreground)
                    }
                }

                Label {
                    Layout.row: 1
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: trackMetaLabel
                    text: "."
                    color: Contemporary.disabled(Contemporary.foreground)

                    Component.onCompleted: () => {
                        trackMetaLabel.text = Qt.binding(() => {
                            const list = [];
                            if (trackItem.artist !== "") list.push(qsTr("by %1").arg(trackItem.artist));
                            if (trackItem.album !== "") list.push(qsTr("on %1").arg(trackItem.album));

                            if (list.length == 0) return qsTr("Track");
                            return list.join(" · ")
                        })
                    }
                }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: () => {
                           PlaylistManager.addItem(UrlManager.itemForUrl(Qt.url(`file://${trackItem.path}`)))
                       }
        }
    }
}
