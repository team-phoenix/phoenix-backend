#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "backendcommon.h"

#include "producer.h"
#include "qmlinputdevice.h"
#include "sdleventloop.h"
#include "inputdevice.h"
#include "keyboard.h"
#include "logging.h"

class InputManager : public QObject, public Producer {
        Q_OBJECT

        Q_PROPERTY( bool gamepadControlsFrontend READ gamepadControlsFrontend
                    WRITE setGamepadControlsFrontend NOTIFY gamepadControlsFrontendChanged )

        Q_PROPERTY( int count READ count NOTIFY countChanged )

        Q_PROPERTY( QString controllerDBFile MEMBER controllerDBFile NOTIFY controllerDBFileChanged )

    public:

        explicit InputManager( QObject *parent = 0 );
        ~InputManager();

        // One keyboard is reserved for being always active.
        Keyboard *keyboard;

        int count() const;
        int size() const;

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

        InputDevice *at( int index );

        InputDevice *get( const QString name );

        bool eventFilter( QObject *object, QEvent *event );

        void pollStates();

    signals:
        PRODUCER_SIGNALS

        void gamepadControlsFrontendChanged();
        void device( InputDevice *device );
        void deviceAdded( InputDevice *device );
        void incomingEvent( InputDeviceEvent *event );
        void countChanged();
        void controllerDBFileChanged( QString controllerDBFile );

    private:
        QMutex mutex;

        QList<InputDevice *> deviceList;

        SDLEventLoop sdlEventLoop;

        QHash<QString, int> mDeviceNameMapping;

        QString controllerDBFile;

        void installKeyboardFilter();

        void removeKeyboardFilter();

};


#endif // INPUTMANAGER_H
