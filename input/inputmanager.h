#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "backendcommon.h"

#include "controllable.h"
#include "producer.h"

#include "qmlinputdevice.h"
#include "sdleventloop.h"
#include "inputdevice.h"
#include "keyboard.h"

#include "logging.h"

/*
 * InputManager is a QObject that manages input devices. Internally it controls the lifecycle of the SDL library and
 * maintains a list of connected controllers.
 */

class InputManager : public QObject, public Producer, public Controllable {
        Q_OBJECT

        Q_PROPERTY( bool gamepadControlsFrontend READ gamepadControlsFrontend
                    WRITE setGamepadControlsFrontend NOTIFY gamepadControlsFrontendChanged )
        Q_PROPERTY( int count READ size NOTIFY countChanged )
        Q_PROPERTY( QString controllerDBFile MEMBER controllerDBFile NOTIFY controllerDBFileChanged )

    public:
        explicit InputManager( QObject *parent = 0 );
        ~InputManager();

        static void registerTypes();

    public slots:
        void setState( Control::State currentState ) override;

        // Allows the user to change controller ports
        void swap( const int index1, const int index2 );

        // Iterate through, and expose inputDevices to QML
        // FIXME: Remove this, replace with property
        void emitConnectedDevices();

        // Fetch an InputDevice by index into the list
        InputDevice *at( int index );

        // Fetch an InputDevice by the device name used by SDL
        InputDevice *get( const QString name );

        bool eventFilter( QObject *object, QEvent *event ) override;

        void libretroGetInputState();

        // Called by the SDLEventLoop

        // Insert or append an inputDevice to the deviceList.
        void insert( InputDevice *device );

        // Remove and delete inputDevice at index
        void removeAt( int index );

    signals:
        PRODUCER_SIGNALS

        void gamepadControlsFrontendChanged();
        void deviceAdded( InputDevice *device );
        void incomingEvent( InputDeviceEvent *event );
        void countChanged();
        void controllerDBFileChanged( QString controllerDBFile );

        // FIXME: Where is this used?
        // void device( InputDevice *device );

    private:
        // The internal list of available input devices
        // One keyboard is reserved for being always active and is guarantied to be at gamepadList[ 0 ]
        Keyboard *keyboard;
        QList<InputDevice *> gamepadList;

        // Convenient getter for getting gamepad list size
        int size() const;

        bool gamepadControlsFrontend() const;

        // This is just a wrapper around InputDevice::gamepadControlsFrontend
        // Global property that affects all gamepads
        void setGamepadControlsFrontend( const bool control );

        QMutex mutex;

        SDLEventLoop sdlEventLoop;

        QHash<QString, int> gamepadNameMapping;

        QString controllerDBFile;

        void installKeyboardFilter();

        void removeKeyboardFilter();

        // Helper ripped from old Core input state callback
        // TODO: Support more than 16 RETRO_DEVICE_JOYPADs
        int16_t inputStates[ 16 ];
        int16_t libretroGetInputStateHelper( unsigned controllerPort, unsigned retroDeviceType, unsigned analogIndex, unsigned buttonID );

};


#endif // INPUTMANAGER_H
