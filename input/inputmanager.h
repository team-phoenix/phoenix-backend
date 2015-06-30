#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QObject>
#include <QList>
#include <QEvent>
#include <QKeyEvent>

#include "input/sdleventloop.h"
#include "input/inputdevice.h"
#include "input/keyboard.h"
#include "logging.h"

#include <memory>

class InputManager : public QObject {
        Q_OBJECT

        Q_PROPERTY( bool gamepadControlsFrontend READ gamepadControlsFrontend
                    WRITE setGamepadControlsFrontend NOTIFY gamepadControlsFrontendChanged )

    public:

        explicit InputManager( QObject *parent = 0 );
        ~InputManager();

        // One keyboard is reserved for being always active.
        Keyboard *keyboard;

        int size() const;

        InputDevice *at( int index );

        void pollStates();

        bool gamepadControlsFrontend() const;

        // This is just a wrapper around InputDevice::gamepadControlsFrontend.
        void setGamepadControlsFrontend( const bool control );

        static void registerTypes();


    public slots:

        // Insert or append an inputDevice to the deviceList.
        void insert( InputDevice *device );

        // Remove and delete inputDevice at index.
        void removeAt( int index );

        // Handle when the game has started playing.
        void setRun( bool run );

        // Allows the user to change controller ports.
        void swap( const int index1, const int index2 );

        // Iterate through, and expose inputDevices to QML.
        void emitConnectedDevices();

    signals:

        void gamepadControlsFrontendChanged();
        void device( InputDevice *device );
        void deviceAdded( InputDevice *device );
        void incomingEvent( InputDeviceEvent *event );

    private:

        QMutex mutex;

        QList<InputDevice *> deviceList;

        SDLEventLoop sdlEventLoop;


};


#endif // INPUTMANAGER_H
