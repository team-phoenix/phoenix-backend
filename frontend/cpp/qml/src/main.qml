import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: root;
    visible: true;
    width: 640;
    height: 480;
    minimumHeight: 480;
    minimumWidth: 640;
    title: qsTr("Phoenix");

    GameListView {
        id: gameListView;
        anchors {
            fill: parent;
        }
    }

}
