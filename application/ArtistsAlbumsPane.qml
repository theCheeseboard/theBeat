import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import com.vicr123.thebeat
import com.vicr123.thebeat.library

Item {
    id: root

    required property bool isArtists

    QtObject {
        id: d

        property string lookup
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

                text: root.isArtists ? qsTr("Artists in Library") : qsTr("Albums in Library")
                color: layer1.color
            }

            ListView {
                id: mainList
                Layout.fillWidth: true
                Layout.fillHeight: true

                delegate: ItemDelegate {
                    required property string modelData

                    text: modelData
                    width: parent.width
                    onClicked: () => {
                        d.lookup = modelData;
                        pager.currentIndex = 1;
                    }
                }
            }

            Component.onCompleted: () => {
                mainList.model = root.isArtists ? LibraryManager.artists() : LibraryManager.albums();
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

                text: root.isArtists ? qsTr("Tracks by %1").arg(Contemporary.quoteString(d.lookup)) : qsTr("Tracks in %1").arg(Contemporary.quoteString(d.lookup))
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

                Component.onCompleted: () => {
                    trackList.model = Qt.binding(() => root.isArtists ? LibraryManager.tracksByArtist(d.lookup) : LibraryManager.tracksByAlbum(d.lookup));
                }
            }
        }
    }
}
