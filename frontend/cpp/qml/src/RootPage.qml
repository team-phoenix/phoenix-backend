import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import vg.phoenix.models 1.0

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
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
                    text: "Add Game"
                    onClicked: findGameDialog.open()
                }

                ComboBox {
                    textRole: "systemFullName"
                    model: SystemModel {
                    }
                    onCurrentTextChanged: {
                        globalGameMetadataModel.filterBySystem(currentText)
                    }
                }
            }
        }

        GameListView {
            id: gameListView
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
