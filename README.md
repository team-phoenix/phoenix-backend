# phoenix-backend
A Libretro frontend encapsulated in a QML object.

This is the backend repositiory for the [**Phoenix**](https://github.com/team-phoenix/Phoenix) frontend.

###Example Usage

####From C++
```c++
#include <QApplication>
#include <QQmlApplicationEngine>

int main( int argc, char *argv[] ) {

    QApplication app( argc, argv );

    QQmlApplicationEngine engine;

    // Necessary to quit properly
    QObject::connect( &engine, &QQmlApplicationEngine::quit, &app, &QApplication::quit );

    // Make C++ classes visible to QML
    VideoItem::registerTypes();
    InputManager::registerTypes();

    engine.load( QUrl( QString( "qrc:/main.qml" ) ) );

    return app.exec();
}
```

###From QML
```qml
import vg.phoenix.backend 1.0

ApplicationWindow {
    id: root;
    
    VideoItem {
        // This is a rendering surface for the emulator.
        anchors.fill: parent;
        inputManager: globalInputManager;
    }
    
    InputManger {
        // Controls the input system; keyboards and SDL2 compatible game controllers.
        id: globalInputManager;
        
        onDeviceAdded: {
            device.inputDeviceEvent.connect( qmlInputDevice.insert );
        }
    }
    
    QMLInputDevice {
        // Controls the UI via a game controller.
        id: qmlInputDevice;
    }

}
```
