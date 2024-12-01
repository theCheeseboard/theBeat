import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import com.vicr123.Contemporary
import com.vicr123.thebeat

Item {
    property var source
    readonly property var controller: source.controller
    readonly property var musicBrainz: controller.musicBrainzClient

    LayerCalculator {
        id: layer1
        layer: 1
    }

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

                Layer {
                    Layout.fillWidth: true
                    color: layer1.color

                    visible: musicBrainz?.loading && musicBrainz?.supported
                    implicitHeight: childrenRect.height + 20

                    RowLayout {
                        x: 10
                        y: 10

                        BusyIndicator {
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Querying MusicBrainz CD Database...")
                        }
                    }
                }

                Layer {
                    Layout.fillWidth: true
                    color: layer1.color

                    visible: !musicBrainz?.loading && musicBrainz?.supported && releaseSelectionBox.count > 1
                    implicitHeight: childrenRect.height + 20

                    ColumnLayout {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        anchors.topMargin: 10

                        SubtitleLabel {
                            Layout.fillWidth: true
                            text: qsTr("Select Correct Album")
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("More than one match was found on MusicBrainz for this CD. Select the correct album for accurate track information.")
                            wrapMode: Text.WordWrap
                        }

                        ComboBox {
                            id: releaseSelectionBox
                            Layout.fillWidth: true
                            model: musicBrainz
                            textRole: "comboBoxLabel"
                            valueRole: "releaseId"
                            currentIndex: indexOfValue(musicBrainz?.selectedReleaseId)

                            onCurrentValueChanged: () => {
                                musicBrainz.selectMusicbrainzRelease(releaseSelectionBox.currentValue);
                            }

                            delegate: ItemDelegate {
                                id: releaseDelegate

                                required property string releaseTitle
                                required property string releaseDate
                                required property string releaseBarcode
                                required property string releaseCountry
                                required property string releaseId

                                height: delegateLayout.implicitHeight + 20
                                width: releaseSelectionBox.width

                                ColumnLayout {
                                    x: 10
                                    y: 10
                                    id: delegateLayout

                                    Label {
                                        text: releaseDelegate.releaseTitle
                                        font.pointSize: 15
                                    }
                                    Label {
                                        text: `Released: ${releaseDelegate.releaseDate}`
                                    }
                                    Label {
                                        text: `Barcode: ${releaseDelegate.releaseBarcode}`
                                    }
                                    Label {
                                        text: `Country: ${releaseDelegate.releaseCountry}`
                                    }
                                }
                            }
                        }
                    }
                }

                Layer {
                    Layout.fillWidth: true
                    color: layer1.color

                    visible: !musicBrainz?.loading && musicBrainz?.supported && releaseSelectionBox.count == 0
                    implicitHeight: childrenRect.height + 20

                    RowLayout {
                        x: 10
                        y: 10

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("This CD was not found on the MusicBrainz CD database")
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
