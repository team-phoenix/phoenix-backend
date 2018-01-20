import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import vg.phoenix.emulation 1.0

Rectangle {
    id: gamePage
    color: "violet"

    Component.onDestruction: {
        root.title = "Phoenix"
    }

    EmulationVideoScreen {
        id: emulationVideoScreen;
        anchors {
            left: parent.left;
            right: parent.right;
            top: parent.top;
        }

        height: 100;
    }

    Rectangle {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 24
        }

        height: 36

        color: "black"

        RowLayout {
            anchors.fill: parent
            Button {
                text: qsTr("Quit!")
                onClicked: {
                    if (root.visibility === ApplicationWindow.FullScreen) {
                        root.visibility = ApplicationWindow.Windowed
                    }
                    rootStackView.pop()
                }
            }

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
