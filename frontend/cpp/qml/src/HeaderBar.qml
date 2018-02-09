import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Rectangle {
    id: headerBar
    height: 50
    color: "cyan"

    RowLayout {
        id: buttonRowLayout
        anchors.centerIn: parent

        Button {
            text: qsTr("Games")

            onClicked: {
                rootStackView.setCurrentItem(rootPage)
            }
        }

        Button {
            text: qsTr("Input")

            onClicked: {
                rootStackView.setCurrentItem(inputRemappingPage)
            }
        }
    }
}
