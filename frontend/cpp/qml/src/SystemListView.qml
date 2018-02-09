import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import vg.phoenix.models 1.0

ListView {
    id: systemListView

    Rectangle {
        anchors.fill: parent
        z: parent.z - 1
        color: "orange"
    }

    model: SystemModel {
    }

    delegate: Item {
        height: 50
        width: parent.width

        property bool isSelectedItem: index == systemListView.currentIndex
        onIsSelectedItemChanged: {
            if (isSelectedItem) {
                globalGameMetadataModel.filterBySystem(systemFullName)
            }
        }

        Rectangle {
            anchors.fill: parent
            color: isSelectedItem ? "pink" : "gray"

            Text {
                text: qsTr(systemFullName)
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: 12
                }
                font {
                    bold: true
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: systemListView.currentIndex = index
        }
    }
}
