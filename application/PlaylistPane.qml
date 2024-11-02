import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import com.vicr123.thebeat
import com.vicr123.thebeat.library

Item {
    id: root

    QtObject {
        id: d

        property int lookup
        property int index
    }

    LayerCalculator {
        id: layer1
        layer: 1
    }

    Pager {
        id: pager
        anchors.fill: parent
        currentAnimation: Pager.Lift

        ColumnLayout {
            anchors.fill: parent

            Grandstand {
                id: grandstand
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                innerTopMargin: SafeZone.top
                z: 20

                text: qsTr("Playlists")
                color: layer1.color
            }

            ListView {
                id: mainList
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: UserPlaylistModel {}

                delegate: ItemDelegate {
                    id: listItem
                    required property int id
                    required property var model
                    required property string name
                    required property int index

                    text: name
                    width: parent.width
                    onClicked: () => {
                        d.lookup = listItem.id;
                        d.index = listItem.index;
                        trackList.model = listItem.model;
                        pager.currentIndex = 1;
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent

            LibraryHeader {
                id: grandstand2
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                innerTopMargin: SafeZone.top
                backButtonVisible: true
                z: 20

                text: {
                    return mainList.model.data(mainList.model.index(d.index, 0));
                }

                color: layer1.color

                onBackButtonClicked: () => {
                    pager.currentIndex = 0;
                }

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
                }
            }

            LibraryListing {
                id: trackList
                Layout.fillHeight: true
                Layout.fillWidth: true

                function enqueueAll() {
                    for (var i = 0; i < trackList.model.rowCount(); i++) {
                        const mediaItem = UrlManager.itemForUrl(trackList.model.path(i));
                        PlaylistManager.addItem(mediaItem);
                    }
                }
            }
        }
    }
}
