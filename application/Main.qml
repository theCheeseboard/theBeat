import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import Qt.labs.platform as Labs
import com.vicr123.Contemporary.CoreStyles
import com.vicr123.thebeat

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
                shortcut: hk_`Ctrl+O`
                text: qsTr("Open File")
                onTriggered: () => {}
            }
            Labs.MenuItem {
                text: qsTr("Open URL")
                onTriggered: () => {}
            }
            Labs.MenuSeparator {}
            Labs.MenuItem {
                text: qsTr("Add to Library")
                onTriggered: () => {}
            }
            Labs.MenuSeparator {}
            Labs.MenuItem {
                shortcut: hk_`Ctrl+P`
                text: qsTr("Print")
                onTriggered: () => {}
            }
        }
        Labs.Menu {
            title: qsTr("Playback")

            Labs.MenuItem {
                text: qsTr("Play")
                shortcut: hk_`Space`
                onTriggered: () => PlaylistManager.playPause()
                enabled: !!PlaylistManager.currentItem
            }
            Labs.MenuItem {
                text: qsTr("Skip Back")
                shortcut: hk_`Shift+Left`
                onTriggered: () => PlaylistManager.previous()
                enabled: !!PlaylistManager.currentItem
            }
            Labs.MenuItem {
                text: qsTr("Skip Forward")
                shortcut: hk_`Shift+Right`
                onTriggered: () => PlaylistManager.next()
                enabled: !!PlaylistManager.currentItem
            }
            Labs.MenuSeparator {}
            Labs.MenuItem {
                text: qsTr("Increase Volume")
                shortcut: hk_`Up`
                onTriggered: () => {
                    const newVolume = PlaylistManager.volume;
                    newVolume += 0.1;
                    if (newVolume > 1)
                        newVolume = 1;
                    PlaylistManager.volume = newVolume;
                }
            }
            Labs.MenuItem {
                text: qsTr("Decrease Volume")
                shortcut: hk_`Down`
                onTriggered: () => {
                    const newVolume = PlaylistManager.volume;
                    newVolume -= 0.1;
                    if (newVolume < 0)
                        newVolume = 0;
                    PlaylistManager.volume = newVolume;
                }
            }
            Labs.MenuSeparator {}
            Labs.MenuItem {
                id: repeatOneMenuItem
                text: qsTr("Repeat One")
                shortcut: hk_`Ctrl+R`
                checked: PlaylistManager.repeatOne
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.repeatOne = repeatOneMenuItem.checked;
                }
            }
            Labs.MenuItem {
                id: shuffleMenuItem
                text: qsTr("Shuffle")
                shortcut: hk_`Ctrl+S`
                checked: PlaylistManager.shuffle
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.shuffle = shuffleMenuItem.checked;
                }
            }
            Labs.MenuSeparator {}
            Labs.MenuItem {
                id: pauseAfterCurrentTrackMenuItem
                text: qsTr("Pause after current track")
                shortcut: hk_`Shift+Escape`
                checked: PlaylistManager.pauseAfterCurrentTrack
                checkable: true
                onCheckedChanged: () => {
                    PlaylistManager.pauseAfterCurrentTrack = pauseAfterCurrentTrackMenuItem.checked;
                }
            }
        }
        Labs.Menu {
            title: qsTr("View")

            Labs.MenuItem {
                text: qsTr("Zen Mode")
                onTriggered: () => {}
            }
        }
        Labs.Menu {
            title: qsTr("Help")

            Labs.MenuItem {
                shortcut: hk_`F1`
                text: qsTr("theBeat Help")
                onTriggered: () => {}
            }
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
                        PlaylistPane {}
                        OtherSourcesPane {}
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
