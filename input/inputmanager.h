#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "backendcommon.h"

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

class InputManager : public QObject, public Producer {
        Q_OBJECT

        Q_PROPERTY( bool gamepadControlsFrontend READ gamepadControlsFrontend
                    WRITE setGamepadControlsFrontend NOTIFY gamepadControlsFrontendChanged )
        Q_PROPERTY( int count READ count NOTIFY countChanged )
        Q_PROPERTY( QString controllerDBFile MEMBER controllerDBFile NOTIFY controllerDBFileChanged )

    public:
        explicit InputManager( QObject *parent = 0 );
        ~InputManager();

        // One keyboard is reserved for being always active
        Keyboard *keyboard;

        int count() const;
        int size() const;

        bool gamepadControlsFrontend() const;

        // This is just a wrapper around InputDevice::gamepadControlsFrontend
        void setGamepadControlsFrontend( const bool control );

        static void registerTypes();

    public slots:
        // Handle when the game has started playing
        void setRun( bool run );

        // Allows the user to change controller ports
        void swap( const int index1, const int index2 );

        // Iterate through, and expose inputDevices to QML
        // FIXME: Remove this, replace with property
        void emitConnectedDevices();

        // Fetch an InputDevice by index into the list
        InputDevice *at( int index );

        // Fetch an InputDevice by the device name used by SDL
        InputDevice *get( const QString name );

        bool eventFilter( QObject *object, QEvent *event );

        void pollStates();

        // Polls per second
        void setPollRate( qreal rate );

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
        QMutex mutex;

        QList<InputDevice *> deviceList;

        SDLEventLoop sdlEventLoop;

        QHash<QString, int> mDeviceNameMapping;

        QString controllerDBFile;

        void installKeyboardFilter();

        void removeKeyboardFilter();

        qreal pollRate;

        // Helper ripped from old Core input state callback
        int16_t inputStates[ 16 ];
        int16_t getInputState( unsigned controllerPort, unsigned retroDeviceType, unsigned analogIndex, unsigned buttonID );

};


#endif // INPUTMANAGER_H
