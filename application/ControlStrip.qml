import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import com.vicr123.thebeat
import Contemporary

Layer {
    id: root
    implicitHeight: PlaylistManager.currentItem ? rootLayout.implicitHeight + 18 + SafeZone.bottom : 0
    color: layer1.color

    LayerCalculator {
        id: layer1
        layer: 1
    }

    ColumnLayout {
        id: rootLayout
        x: 9
        y: 9
        width: root.width - 18

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Label {
                    text: PlaylistManager.currentItem.title
                    font.pointSize: 20
                }
                Label {
                    id: currentItemMeta
                    text: ""

                    Component.onCompleted: () => {
                        currentItemMeta.text = Qt.binding(() => {
                            const list = [];
                            if (PlaylistManager.currentItem.authors.length !== 0) list.push(Contemporary.createSeparatedList(PlaylistManager.currentItem.authors))
                            if (PlaylistManager.currentItem.album !== "") list.push(PlaylistManager.currentItem.album)
                            return list.join(" · ")
                        })
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: shuffleButton
                flat: true
                icon.name: "media-playlist-shuffle"
                implicitWidth: height
                checked: PlaylistManager.shuffle
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.shuffle = shuffleButton.checked
                }
            }
            Button {
                id: repeatOneButton
                flat: true
                icon.name: "media-repeat-single"
                implicitWidth: height
                checked: PlaylistManager.repeatOne
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.repeatOne = repeatOneButton.checked
                }
            }
            Button {
                flat: true
                icon.name: "media-skip-backward"
                implicitWidth: height
                onClicked: () => {
                    PlaylistManager.previous();
                }
            }
            Button {
                flat: true
                icon.name: PlaylistManager.state === PlaylistManager.Playing ? "media-playback-pause" : "media-playback-start"
                icon.height: 32
                icon.width: 32
                implicitWidth: height
                onClicked: () => {
                    PlaylistManager.playPause();
                }
            }
            Button {
                flat: true
                icon.name: "media-skip-forward"
                implicitWidth: height
                onClicked: () => {
                    PlaylistManager.next();
                }
            }
        }
    }
}
