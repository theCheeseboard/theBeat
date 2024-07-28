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

    Grandstand {
        id: grandstand
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        innerTopMargin: SafeZone.top
        z: 20

        text: qsTr("Tracks in Library")
        color: layer1.color
    }

    LibraryModel {
        id: model
    }

    Pager {
        anchors.top: grandstand.bottom
        anchors.topMargin: 6
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: SafeZone.bottom

        currentAnimation: Pager.Fade

        LibraryListing {
            anchors.fill: parent
            id: trackList
        }
    }

    Component.onCompleted: () => {
        trackList.model = LibraryManager.allTracks()
    }
}
