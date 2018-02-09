import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

ListView {
    id: listView

    Connections {
        id: globalInputDeviceConnection
        target: globalInputDevice
        onButtonUpChanged: {
            if (globalInputDevice.buttonUp) {
                listView.decrementCurrentIndex()
            }
        }

        onButtonDownChanged: {
            if (globalInputDevice.buttonDown) {
                listView.incrementCurrentIndex()
            }
        }

        onButtonAChanged: {
            if (globalInputDevice.buttonA) {
                listView.currentItem.listItemPlayButton.clicked()
            }
        }
    }

    model: globalGameMetadataModel
    clip: true

    orientation: ListView.Vertical
    spacing: 12

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AlwaysOn
        //        onPositionChanged: {
        //            var scaledHeight = visibleArea.heightRatio + position
        //            console.log(scaledHeight)
        //            if (scaledHeight === 1.0) {
        //                listViewBottomShadow.visible = false
        //            } else {
        //                if (!listViewBottomShadow.visible) {
        //                    listViewBottomShadow.visible = true
        //                }
        //            }
        //        }
    }

    //    Rectangle {
    //        id: listViewBottomShadow
    //        anchors {
    //            bottom: parent.bottom
    //            left: parent.left
    //            right: parent.right
    //        }

    //        height: 12
    //        gradient: Gradient {
    //            GradientStop {
    //                position: 0.0
    //                color: "transparent"
    //            }
    //            GradientStop {
    //                position: 1.0
    //                color: "#292928"
    //            }
    //        }
    //    }
    delegate: Rectangle {
        id: listViewDelegate

        height: 200
        width: parent.width
        color: index === listView.currentIndex ? "white" : "black"

        property alias listItemPlayButton: playButton

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                rightClickMenu.popup()
            }
            onPressAndHold: {
                if (mouse.source === Qt.MouseEventNotSynthesized) {
                    rightClickMenu.popup()
                }
            }

            Menu {
                id: rightClickMenu
                MenuItem {
                    text: qsTr("Remove")
                    onClicked: listView.model.removeGameAt(index)
                }
            }
        }

        RowLayout {
            anchors {
                fill: parent
                margins: spacing
            }

            spacing: 24

            Image {
                width: 150
                anchors {
                    left: parent.left
                }
                sourceSize {
                    height: 150
                    width: 150
                }

                fillMode: Image.PreserveAspectFit
                Layout.fillHeight: true
                source: gameImageSource
            }

            Item {
                id: gameMetaSection
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    id: col
                    anchors {
                        fill: parent
                    }

                    Text {
                        text: qsTr(gameTitle)
                        font {
                            pixelSize: 24
                            bold: true
                        }
                        Layout.fillWidth: true
                        color: "white"
                        elide: Text.ElideRight
                        Rectangle {
                            anchors.fill: parent
                            color: "black"
                            z: parent.z - 1
                        }
                    }

                    Text {
                        text: qsTr(gameSystem)
                        font {
                            bold: true
                            pixelSize: 12
                        }
                        Layout.fillWidth: true
                        color: "white"
                        Rectangle {
                            anchors.fill: parent
                            color: "black"
                            z: parent.z - 1
                        }
                    }

                    TextArea {
                        id: descriptionArea
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        selectByMouse: true
                        text: gameDescription
                        wrapMode: TextArea.Wrap

                        Rectangle {
                            color: "green"
                            anchors.fill: parent
                            z: parent.z - 1
                        }
                    }

                    Rectangle {
                        id: gameActionSection
                        color: "pink"
                        height: playButton.height
                        Layout.fillWidth: true
                        anchors {
                            bottom: parent.bottom
                        }

                        RowLayout {
                            Button {
                                id: playButton
                                text: qsTr("Play")
                                onClicked: {
                                    if (globalEmulationListener.sendPlayMessage(
                                                gameAbsoluteFilePath,
                                                gameSystem)) {
                                        rootStackView.setCurrentItem(gamePage)
                                        root.title = "Phoenix: " + gameTitle
                                    }
                                }
                            }

                            Button {
                                text: qsTr("Details")
                            }
                        }
                    }
                }
            }
        }
    }
}
