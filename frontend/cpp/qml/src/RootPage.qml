import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {

            Layout.fillHeight: true
            Layout.fillWidth: true

            spacing: 0

            ColumnLayout {
                Layout.fillHeight: true
                width: 200
                spacing: 0

                SystemListView {
                    id: systemListView
                    Layout.fillHeight: true
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                }

                Rectangle {
                    color: "violet"
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    height: 75

                    FileDialog {
                        id: findGameDialog
                        selectFolder: false
                        selectMultiple: true
                        onAccepted: {
                            globalGameMetadataModel.importGames(findGameDialog.fileUrls)
                        }
                    }

                    Button {
                        anchors.centerIn: parent
                        text: qsTr("Add Game")
                        onClicked: {
                            console.log("cli")
                            findGameDialog.open()
                        }
                    }
                }
            }

            GameListView {
                id: gameListView
                Layout.fillHeight: true
                Layout.fillWidth: true

                Rectangle {
                    anchors.fill: parent
                    z: parent.z - 1
                    color: "yellow"
                }
            }
        }
    }
}
