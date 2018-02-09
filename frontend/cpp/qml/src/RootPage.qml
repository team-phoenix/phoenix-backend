import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: headerBar
            Layout.fillWidth: true
            height: 50
            color: "cyan"

            FileDialog {
                id: findGameDialog
                selectFolder: false
                selectMultiple: true
                onAccepted: {
                    globalGameMetadataModel.importGames(findGameDialog.fileUrls)
                }
            }

            RowLayout {
                anchors.centerIn: parent

                Button {
                    text: qsTr("Add Game")
                    onClicked: {
                        console.log("cli")
                        findGameDialog.open()
                    }
                }

                Button {
                    text: qsTr("Remap Input")
                    onClicked: {
                        rootStackView.push(inputRemappingPage)
                    }
                }
            }
        }

        RowLayout {

            Layout.fillHeight: true
            Layout.fillWidth: true

            spacing: 0

            SystemListView {
                id: systemListView
                Layout.fillHeight: true

                width: 200
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
