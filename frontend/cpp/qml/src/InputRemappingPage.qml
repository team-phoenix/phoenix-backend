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
            id: inputDeviceComboBox
            textRole: "inputDeviceDisplayName"
            implicitWidth: 200
            model: InputDeviceInfoModel {
            }

            onCountChanged: inputDeviceComboBox.currentIndex = 0
            onCurrentIndexChanged: {
                var currentMapping = inputDeviceComboBox.model.getInputMapping(
                            currentIndex)
                inputMappingListView.model.setCurrentMapping(currentMapping)
            }
        }

        ListView {
            id: inputMappingListView
            clip: true

            Rectangle {
                anchors.fill: parent
                color: "green"
                z: parent.z - 1
            }

            model: InputMappingModel {
            }

            Layout.fillHeight: true
            Layout.fillWidth: true

            delegate: Item {
                height: 35
                width: inputMappingListView.width

                property bool isSelected: index === inputMappingListView.currentIndex

                Rectangle {
                    anchors.fill: parent
                    color: isSelected ? "violet" : "teal"

                    RowLayout {

                        width: 200

                        anchors {
                            centerIn: parent
                        }

                        Rectangle {
                            anchors {
                                fill: parent
                            }

                            z: parent.z - 1
                            color: "orange"
                        }

                        Text {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                            }

                            text: mappingKey
                        }

                        Rectangle {
                            width: 100
                            Layout.fillHeight: true
                            color: "yellow"

                            Text {
                                anchors {
                                    centerIn: parent
                                }

                                text: mappingValue
                            }
                        }
                    }
                }
            }
        }
    }
}
