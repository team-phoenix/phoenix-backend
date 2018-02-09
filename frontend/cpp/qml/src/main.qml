import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: root
    visible: true
    width: 640
    height: 480
    minimumHeight: 480
    minimumWidth: 640
    title: qsTr("Phoenix")

    Component {
        id: inputRemappingPage

        InputRemappingPage {
        }
    }

    Component {
        id: rootPage

        RootPage {
        }
    }

    Component {
        id: gamePage

        GamePage {
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        HeaderBar {
            id: headerBar
            Layout.fillWidth: true
            height: 50
        }

        StackView {
            id: rootStackView
            Layout.fillHeight: true
            Layout.fillWidth: true

            Component.onCompleted: {
                setCurrentItem(rootPage)
            }

            function setCurrentItem(item) {
                if (pushedComponents.length > 0) {
                    if (pushedComponents[0] !== item) {
                        rootStackView.pop()
                        pushedComponents.pop()

                        rootStackView.push(item)
                        pushedComponents.push(item)
                    }
                } else {
                    rootStackView.push(item)
                    pushedComponents.push(item)
                }
            }

            property var pushedComponents: []
        }
    }
}
