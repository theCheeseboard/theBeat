import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import com.vicr123.thebeat
import com.vicr123.thebeat.library

Item {
    id: root

    LayerCalculator {
        id: layer1
        layer: 1
    }

    LibraryHeader {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        innerTopMargin: SafeZone.top
        z: 20

        text: qsTr("Tracks in Library")
        color: layer1.color

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            Button {
                text: qsTr("Enqueue All")
                icon.name: "view-media-playlist"

                onClicked: () => {
                    for (var i = 0; i < trackList.model.rowCount(); i++) {
                        const mediaItem = UrlManager.itemForUrl(trackList.model.path(i));
                        PlaylistManager.addItem(mediaItem);
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }

    Pager {
        anchors.top: grandstand.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentAnimation: Pager.Fade

        ColumnLayout {
            anchors.fill: parent

            TextField {
                id: searchBox
                Layout.fillWidth: true
                placeholderText: qsTr("Search")

                background: Rectangle {
                    implicitHeight: 40
                    color: Contemporary.background
                }

                z: 1
            }

            LibraryListing {
                id: trackList
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }

    Component.onCompleted: () => {
        trackList.model = Qt.binding(() => {
            if (searchBox.text !== "") {
                return LibraryManager.searchTracks(searchBox.text);
            }
            return LibraryManager.allTracks();
        });
    }
}
