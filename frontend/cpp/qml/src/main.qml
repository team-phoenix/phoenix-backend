import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import vg.phoenix.models 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 640
    height: 480
    minimumHeight: 480
    minimumWidth: 640
    title: qsTr("Phoenix")
    ColumnLayout {
        anchors {
            fill: parent
        }

        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "cyan"

            FileDialog {
                id: findGameDialog
                selectFolder: false
                selectMultiple: true
                onAccepted: {
                    globalGameMetadataModel.importGames(findGameDialog.fileUrls);
                }
            }

            RowLayout {
                anchors.centerIn: parent

                Button {
                    text: "Add Game"
                    onClicked: findGameDialog.open()
                }

                ComboBox {
                    textRole: "systemShortName"
                    model: SystemModel {
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
