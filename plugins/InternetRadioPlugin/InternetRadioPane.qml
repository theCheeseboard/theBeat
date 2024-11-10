import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import com.vicr123.thebeat
import Contemporary

Item {
    property var source

    StationListModel {
        id: stationListModel
    }

    Pager {
        anchors.fill: parent

        Item {
            Grandstand {
                id: title

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                innerTopMargin: SafeZone.top
                text: qsTr("Internet Radio")
                z: 10
            }

            ColumnLayout {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: title.bottom

                TextField {
                    id: searchBox
                    Layout.fillWidth: true
                    placeholderText: qsTr("Search")

                    text: stationListModel.searchQuery
                    onTextEdited: stationListModel.searchQuery = searchBox.text

                    background: Rectangle {
                        implicitHeight: 40
                        color: Contemporary.background
                    }

                    z: 1
                }

                ListView {
                    id: stationList

                    displayMarginBeginning: SafeZone.top
                    displayMarginEnd: SafeZone.bottom
                    focus: true
                    spacing: 3

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    highlight: Item {
                        Rectangle {
                            anchors.fill: parent
                            anchors.leftMargin: 3
                            anchors.rightMargin: 3
                            color: Contemporary.accent
                            radius: 4
                        }
                    }
                    model: stationListModel
                    delegate: Component {
                        Layer {
                            id: radioItem
                            required property string name
                            required property string country
                            required property url url

                            implicitHeight: childrenRect.height + 6
                            implicitWidth: stationList.width
                            clip: true

                            Label {
                                id: radioName

                                anchors.left: parent.left
                                anchors.leftMargin: 6
                                anchors.right: playButton.left
                                anchors.rightMargin: 6
                                anchors.top: parent.top
                                anchors.topMargin: 3
                                text: radioItem.name
                            }
                            Label {
                                id: radioCountry

                                anchors.left: parent.left
                                anchors.leftMargin: 6
                                anchors.right: playButton.left
                                anchors.rightMargin: 6
                                anchors.top: radioName.bottom
                                anchors.topMargin: 3
                                color: Contemporary.disabled(Contemporary.foreground)
                                text: radioItem.country
                            }

                            Button {
                                id: playButton

                                anchors.right: parent.right
                                anchors.rightMargin: 6
                                anchors.verticalCenter: parent.verticalCenter
                                icon.name: "media-playback-start"
                                implicitWidth: height
                                onClicked: () => {
                                    const mediaItem = UrlManager.itemForUrl(radioItem.url);
                                    PlaylistManager.addItem(mediaItem);
                                    PlaylistManager.currentItem = mediaItem;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
