import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import vg.phoenix.models 1.0

Item {
    id: inputRemappingPage

    property bool remappingModeEnabled: true

    Component.onCompleted: {
        globalEmulationListener.getInputInfoList()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            anchors {
                top: parent.top
            }

            height: 50
            color: "red"

            Button {
                anchors {
                    verticalCenter: parent.verticalCenter
                }

                text: qsTr("Back")
                onClicked: rootStackView.pop()
            }
        }

        ComboBox {
            id: inputDeviceComboBox;
            textRole: "inputDeviceName"
            model: InputDeviceInfoModel {
            }

            onCountChanged: inputDeviceComboBox.currentIndex = 0;
        }

        ListView {
            id: inputMappingListView
            clip: true

            Rectangle {
                anchors.fill: parent
                color: "green"
                z: parent.z - 1
            }

            model: ["hello", "baby", "woo"]

            Layout.fillHeight: true
            Layout.fillWidth: true

            delegate: Item {
                height: 35
                width: inputMappingListView.width

                property bool isSelected: index === inputMappingListView.currentIndex

                Rectangle {
                    anchors.fill: parent
                    color: isSelected ? "violet" : "teal"

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                    }
                }
            }
        }
    }
}
