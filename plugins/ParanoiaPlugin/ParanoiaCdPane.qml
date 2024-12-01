import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.Contemporary
import com.vicr123.thebeat

Item {
    property var source
    readonly property var controller: source.controller

    Pager {
        anchors.fill: parent

        Item {
            LibraryHeader {
                id: grandstand
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                innerTopMargin: SafeZone.top
                z: 20

                text: controller.albumName
                color: layer1.color

                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    Button {
                        text: qsTr("Enqueue All")
                        icon.name: "view-media-playlist"

                        onClicked: () => trackList.enqueueAll()
                    }
                    Button {
                        text: qsTr("Play All")
                        icon.name: "media-playback-start"

                        onClicked: () => {
                            PlaylistManager.clear();
                            trackList.enqueueAll();
                        }
                    }
                    Button {
                        text: qsTr("Shuffle All")
                        icon.name: "media-playlist-shuffle"

                        onClicked: () => {
                            trackList.enqueueAll();
                            PlaylistManager.shuffle = true;
                            PlaylistManager.next();
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Button {
                        text: qsTr("Eject")
                        icon.name: "media-eject"

                        onClicked: () => {
                            controller.eject();
                        }
                    }
                }
            }

            ColumnLayout {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: grandstand.bottom

                LibraryListing {
                    id: trackList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    model: controller

                    onEnqueueItem: index => {
                        const mediaItem = controller.mediaItem(index);
                        PlaylistManager.addItem(mediaItem);
                        PlaylistManager.currentItem = mediaItem;
                    }

                    function enqueueAll() {
                        for (var i = 0; i < trackList.model.rowCount(); i++) {
                            const mediaItem = controller.mediaItem(i);
                            PlaylistManager.addItem(mediaItem);
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: controller

        function onEjectError() {
            ejectErrorDialog.visible = true;
        }
    }
}
