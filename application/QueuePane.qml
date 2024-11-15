import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.impl
import Contemporary
import com.vicr123.thebeat
import com.vicr123.Contemporary

Item {
    QtObject {
        id: d

        property var contextMenuItem
    }

    LayerCalculator {
        id: layer1
        layer: 1
    }

    LayerCalculator {
        id: layer2
        layer: 2
    }

    LayerCalculator {
        id: layer3
        layer: 3
    }

    Grandstand {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        innerTopMargin: SafeZone.top
        z: 20

        text: qsTr("Queue")
        color: layer3.color
    }

    Pager {
        anchors.top: grandstand.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentAnimation: Pager.Fade

        ColumnLayout {
            anchors.fill: parent
            anchors.topMargin: 3
            anchors.bottomMargin: 3
            clip: true

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    d.contextMenuItem = null;
                    if (mouse.button === Qt.RightButton)
                        contextMenu.popup();
                }
                onPressAndHold: {
                    d.contextMenuItem = null;
                    if (mouse.source === Qt.MouseEventNotSynthesized)
                        contextMenu.popup();
                }
            }

            ListView {
                id: queueList
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 3
                model: PlaylistModel {}

                delegate: Item {
                    id: queueItem
                    required property var mediaItem
                    required property int drawType
                    required property var priorHeaders

                    height: (queueItem.drawType === 2 ? singleRowItem.implicitHeight : multiRowItem.implicitHeight) + 6
                    width: queueList.width

                    Item {
                        id: queueItemChild
                        height: parent.height
                        width: parent.width - 6
                        x: 3

                        property var backgroundColor: mouseArea.containsMouse ? (queueItem.drawType === 2 ? layer2.color : layer3.color) : (queueItem.drawType === 2 ? layer1.color : layer2.color)

                        Rectangle {
                            anchors.fill: parent
                            color: queueItemChild.backgroundColor
                            radius: 4
                        }

                        RowLayout {
                            id: singleRowItem
                            implicitWidth: parent.width - 6
                            visible: queueItem.drawType === 2
                            x: 3
                            y: 3

                            IconLabel {
                                visible: PlaylistManager.currentItem === queueItem.mediaItem
                                Layout.preferredWidth: trackNumberMetrics.advanceWidth
                                alignment: Qt.AlignCenter
                                icon.name: "media-playback-start"
                                icon.height: 16
                                icon.width: 16
                                icon.color: Contemporary.foreground
                            }

                            Label {
                                visible: PlaylistManager.currentItem !== queueItem.mediaItem
                                Layout.preferredWidth: trackNumberMetrics.advanceWidth

                                TextMetrics {
                                    id: trackNumberMetrics
                                    text: "--"
                                }

                                color: Contemporary.disabled(Contemporary.foreground)
                                horizontalAlignment: Qt.AlignHCenter
                                text: queueItem.mediaItem.trackNumber === 0 ? "-" : queueItem.mediaItem.trackNumber
                            }

                            Label {
                                Layout.fillWidth: true
                                text: queueItem.mediaItem.title
                            }
                        }

                        GridLayout {
                            id: multiRowItem
                            implicitWidth: parent.width - 6
                            visible: queueItem.drawType !== 2
                            rows: 2
                            columns: 2
                            rowSpacing: 3
                            columnSpacing: 3
                            x: 3
                            y: 3

                            FontMetrics {
                                id: fontMetrics
                            }

                            Image {
                                Layout.rowSpan: 2
                                Layout.preferredWidth: fontMetrics.height * 2 + multiRowItem.columnSpacing + 6
                                Layout.preferredHeight: fontMetrics.height * 2 + multiRowItem.columnSpacing + 6
                                source: queueItem.mediaItem.albumArtUrl

                                Rectangle {
                                    visible: queueItem.drawType === 3 && PlaylistManager.currentItem === queueItem.mediaItem
                                    anchors.fill: parent
                                    color: Qt.rgba(0, 0, 0, 0.5)

                                    IconLabel {
                                        anchors.fill: parent
                                        alignment: Qt.AlignCenter
                                        icon.name: "media-playback-start"
                                        icon.height: 16
                                        icon.width: 16
                                        icon.color: Qt.rgba(255, 255, 255, 255)
                                    }
                                }
                            }

                            Label {
                                text: queueItem.drawType === 3 ? queueItem.mediaItem.title : queueItem.mediaItem.album
                            }

                            Label {
                                id: multiRowSubText
                                color: Contemporary.disabled(Contemporary.foreground)

                                Component.onCompleted: () => {
                                    multiRowSubText.text = Qt.binding(() => {
                                        if (queueItem.drawType === 3) {
                                            let text = queueItem.mediaItem.album;
                                            if (text === "") {
                                                // @disable-check M105
                                                text = qsTr("Track");
                                            }
                                            return text;
                                        } else {
                                            let text = Contemporary.createSeparatedList(queueItem.mediaItem.authors);
                                            if (text === "") {
                                                text = qsTr("Album");
                                            }
                                            return text;
                                        }
                                    });
                                }
                            }
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            propagateComposedEvents: false
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            onClicked: mouse => {
                                if (mouse.button === Qt.RightButton) {
                                    d.contextMenuItem = queueItem.mediaItem;
                                    contextMenu.popup();
                                } else {
                                    PlaylistManager.currentItem = queueItem.mediaItem;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Menu {
        id: contextMenu

        Instantiator {
            active: !!d.contextMenuItem
            delegate: MenuSection {
                text: qsTr("For %1").arg(Contemporary.quoteString(d.contextMenuItem?.title ?? ""))
                visible: !!d.contextMenuItem
            }

            onObjectAdded: (index, object) => contextMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => contextMenu.removeItem(object)
        }
        Instantiator {
            active: !!d.contextMenuItem
            delegate: MenuItem {
                text: qsTr("Remove from Queue")
                icon.name: "list-remove"
                onClicked: () => {
                    PlaylistManager.removeItem(d.contextMenuItem);
                }
            }

            onObjectAdded: (index, object) => contextMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => contextMenu.removeItem(object)
        }

        MenuSection {
            text: qsTr("For Queue")
        }
        MenuItem {
            text: qsTr("Clear Queue")
            icon.name: "list-remove"
            onClicked: () => {
                PlaylistManager.clear();
            }
        }
    }
}
