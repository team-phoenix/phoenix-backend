import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import vg.phoenix.emulation 1.0

Rectangle {
    id: gamePage
    color: "black"

    Component.onDestruction: {
        if (root.visibility === ApplicationWindow.FullScreen) {
            root.visibility = ApplicationWindow.Windowed
        }
        root.title = "Phoenix"
        rootStackView.setCurrentItem(rootPage)
    }

    EmulationVideoScreen {
        id: emulationVideoScreen

        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        width: height * aspectRatio

        Rectangle {
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: 24
            }

            height: 36

            color: "pink"

            RowLayout {
                anchors.fill: parent

                Button {
                    text: qsTr("Pause")
                    onClicked: {
                        console.log("Doesnt do anything, should pause")
                    }
                }

                Button {
                    text: qsTr("Full Screen")
                    onClicked: {
                        if (root.visibility === ApplicationWindow.FullScreen) {
                            root.visibility = ApplicationWindow.Windowed
                        } else {
                            root.visibility = ApplicationWindow.FullScreen
                        }
                    }
                }
            }
        }
    }
}
