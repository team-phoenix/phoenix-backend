# phoenix-backend
A Libretro frontend encapsulated in a QML object.

This is the backend repositiory for the [**Phoenix**](https://github.com/team-phoenix/Phoenix) frontend.

## Add to Project

####Dependencies
1. [libsamplerate](http://www.mega-nerd.com/SRC/), (optional if only using the **InputManager**)
2. [SDL2](https://www.libsdl.org/download-2.0.php), (optional if only using the **VideoItem**)
```c++
    #In YOUR_PROJECT_NAME.pro
    
    INCLUDEPATH += ../path/to/phoenix-backend
    
    #In order to use the video renderer, you will need to also include libsamplerate
    LIBS += -lsamplerate
    
    #To use the input manager, you will need to include SDL2
    INCLUDEPATH += /SDL2/include
    LIBS += /SDL2/lib
    LIBS += -lSDL2
    
    LIBS += -L../path/to/phoenix-backend -lphoenix-backend
```

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

####From QML
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
