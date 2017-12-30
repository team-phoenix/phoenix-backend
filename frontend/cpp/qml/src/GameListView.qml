import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import vg.phoenix.models 1.0

ListView {
    id: listView;

    model: GameMetadataModel {}

    orientation: ListView.Vertical;
    spacing: 24;

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AlwaysOn;
    }

    delegate: Rectangle {
        id: listViewDelegate;

        height: 300;
        width: parent.width;

        color: "red";

        RowLayout {
            anchors {
                fill: parent;
                margins: spacing;
            }

            spacing: 12;

            Image {
                width: 200;
                anchors {
                    left: parent.left;
                }
                sourceSize {
                    height: 100;
                    width: 200;
                }

                fillMode: Image.PreserveAspectFit;
                Layout.fillHeight: true;
                source: gameImageSource;
            }

            Item {
                id: gameMetaSection;
                Layout.fillWidth: true;
                Layout.fillHeight: true;

                ColumnLayout {
                    id: col;
                    anchors {
                        fill: parent;
                    }

                    Text {
                        text: qsTr(gameTitle);
                        font {
                            pixelSize: 24;
                            bold: true;
                        }
                        Layout.fillWidth: true;
                        color: "white";
                        elide: Text.ElideRight;
                        Rectangle {
                            anchors.fill: parent;
                            color: "black";
                            z: parent.z - 1;
                        }
                    }

                    Text {
                        text: qsTr(gameSystem);
                        font {
                            bold: true;
                            pixelSize: 12;
                        }
                        Layout.fillWidth: true;
                        color: "white";
                        Rectangle {
                            anchors.fill: parent;
                            color: "black";
                            z: parent.z - 1;
                        }
                    }

                    TextArea {
                        id: descriptionArea;
                        Layout.fillWidth: true;
                        Layout.fillHeight: true;
                        selectByMouse: true;
                        text: gameDescription;
                        wrapMode: TextArea.Wrap;

                        Rectangle {
                            color: "green";
                            anchors.fill: parent;
                            z: parent.z - 1;
                        }
                    }

                    Rectangle {
                        id: gameActionSection;
                        color: "pink";
                        height: playButton.height;
                        Layout.fillWidth: true;
                        anchors {
                            bottom: parent.bottom;
                        }

                        RowLayout {
                            Button {
                                id: playButton;
                                text: qsTr("Play");
                            }

                            Button {
                                text: qsTr("Details");
                            }
                        }
                    }
                }
            }
        }
    }
}
