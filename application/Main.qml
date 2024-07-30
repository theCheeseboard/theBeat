import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import Qt.labs.platform as Labs
import com.vicr123.Contemporary.CoreStyles

ContemporaryWindow {
    id: window

    width: 800
    height: 600
    title: qsTr("theBeat")
    visible: true

    function hk_(shortcut) {
        if (Qt.platform.os === "osx") {
            return shortcut[0];
        } else {
            return `Shift+${shortcut[0]}`;
        }
    }

    NativeMenuBar {
        Labs.Menu {
            title: qsTr("File")

            Labs.MenuItem {
                shortcut: hk_`Ctrl+Q`
                text: qsTr("Quit")
                role: Labs.MenuItem.QuitRole
                onTriggered: Qt.quit()
            }
        }
        Labs.Menu {
            title: qsTr("Edit")

            Labs.MenuItem {
                text: qsTr("Copy")
                shortcut: hk_`Ctrl+C`
                onTriggered: stack.pages[stack.currentIndex].copy()
            }
            Labs.MenuItem {
                text: qsTr("Paste")
                shortcut: hk_`Ctrl+V`
                onTriggered: stack.pages[stack.currentIndex].paste()
            }
        }
        Labs.Menu {
            title: qsTr("Help")

            Labs.MenuItem {
                text: qsTr("About")
                role: Labs.MenuItem.AboutRole
                onTriggered: outerStack.push(aboutSurface)
            }
        }
    }

    ContemporaryStackView {
        id: outerStack
        anchors.fill: parent

        currentAnimation: ContemporaryStackView.Animation.Lift

        initialItem: ContemporaryWindowSurface {
            id: surface

            actionBar: ActionBar {
                menu: Menu {
                    Action {
                        shortcut: hk_`Ctrl+C`
                        text: qsTr("Copy")
                        icon.name: "edit-copy"
                        onTriggered: stack.pages[stack.currentIndex].copy()
                    }
                    Action {
                        shortcut: hk_`Ctrl+V`
                        text: qsTr("Paste")
                        icon.name: "edit-paste"
                        onTriggered: stack.pages[stack.currentIndex].paste()
                    }
                }

                ActionBarTabber {
                    ActionBarTabber.Button {
                        text: qsTr("Tracks")
                        icon.name: "view-media-track"
                        checked: stack.currentIndex === 0
                        onActivated: stack.currentIndex = 0
                    }
                    ActionBarTabber.Button {
                        text: qsTr("Artists")
                        icon.name: "view-media-artist"
                        checked: stack.currentIndex === 1
                        onActivated: stack.currentIndex = 1
                    }
                    ActionBarTabber.Button {
                        text: qsTr("Albums")
                        icon.name: "media-album-cover"
                        checked: stack.currentIndex === 2
                        onActivated: stack.currentIndex = 2
                    }
                    ActionBarTabber.Button {
                        text: qsTr("Playlists")
                        icon.name: "view-media-playlist"
                        checked: stack.currentIndex === 3
                        onActivated: stack.currentIndex = 3
                    }
                    ActionBarTabber.Button {
                        text: qsTr("Other Sources")
                        icon.name: "view-list-details"
                        checked: stack.currentIndex === 4
                        onActivated: stack.currentIndex = 4
                    }
                }

                onAboutClicked: () => outerStack.push(aboutSurface)
            }
            overlayActionBar: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 3

                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Pager {
                        id: stack
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        TracksPane {}
                        ArtistsAlbumsPane {
                            isArtists: true
                        }
                        ArtistsAlbumsPane {
                            isArtists: false
                        }
                    }

                    Layer {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 300

                        QueuePane {
                            anchors.fill: parent
                        }
                    }
                }

                ControlStrip {
                    Layout.fillWidth: true
                }
            }
        }

        Component {
            id: aboutSurface
            AboutSurface {}
        }

        Component {
            id: settingsSurface
            Item {}
        }
    }
}
