import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import com.vicr123.thebeat
import Contemporary
import "common.js" as Common

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
            spacing: 6
            Layout.fillWidth: true

            Image {
                Layout.preferredWidth: height
                Layout.preferredHeight: playPauseButton.implicitHeight
                source: PlaylistManager.currentItem.albumArtUrl
            }

            ColumnLayout {
                spacing: 6
                Label {
                    text: PlaylistManager.currentItem.title
                    font.pointSize: 15
                }
                Label {
                    id: currentItemMeta
                    text: ""

                    Component.onCompleted: () => {
                        currentItemMeta.text = Qt.binding(() => {
                            const list = [];
                            if (PlaylistManager.currentItem.authors.length !== 0)
                                list.push(Contemporary.createSeparatedList(PlaylistManager.currentItem.authors));
                            if (PlaylistManager.currentItem.album !== "")
                                list.push(PlaylistManager.currentItem.album);
                            return list.join(" · ");
                        });
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: shuffleButton
                Layout.alignment: Qt.AlignCenter
                flat: true
                icon.name: "media-playlist-shuffle"
                implicitWidth: height
                checked: PlaylistManager.shuffle
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.shuffle = shuffleButton.checked;
                }
            }
            Button {
                id: repeatOneButton
                Layout.alignment: Qt.AlignCenter
                flat: true
                icon.name: "media-repeat-single"
                implicitWidth: height
                checked: PlaylistManager.repeatOne
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.repeatOne = repeatOneButton.checked;
                }
            }
            Button {
                Layout.alignment: Qt.AlignCenter
                flat: true
                icon.name: "media-skip-backward"
                implicitWidth: height
                onClicked: () => {
                    PlaylistManager.previous();
                }
            }
            Button {
                id: playPauseButton
                Layout.alignment: Qt.AlignCenter
                flat: true
                icon.name: PlaylistManager.state === PlaylistManager.Playing ? "media-playback-pause" : "media-playback-start"
                icon.height: 40
                icon.width: 40
                implicitWidth: height
                onClicked: () => {
                    PlaylistManager.playPause();
                }
            }
            Button {
                Layout.alignment: Qt.AlignCenter
                flat: true
                icon.name: "media-skip-forward"
                implicitWidth: height
                onClicked: () => {
                    PlaylistManager.next();
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                id: elapsedText
                text: Common.durationToString(PlaylistManager.currentItem.elapsed)
            }

            Slider {
                id: progressSlider
                Layout.fillWidth: true
                from: 0
                to: PlaylistManager.currentItem.duration
                value: PlaylistManager.currentItem.elapsed

                onMoved: () => {
                    PlaylistManager.currentItem.seek(progressSlider.value);
                }
            }

            Label {
                text: Common.durationToString(PlaylistManager.currentItem.duration, true)
            }
        }
    }
}
