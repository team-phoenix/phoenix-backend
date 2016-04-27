#pragma once

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QHash>

class Joystick;

// The SDLEventLoop's job is to poll for button states and to emit signals as devices are connected or disconnected.

class SDLEventLoop : public QObject {
        Q_OBJECT

    public:
        explicit SDLEventLoop( QObject *parent = 0 );
        ~SDLEventLoop();

        bool isInitialized() const;

    public slots:
        void pollEvents();

        // FIXME: Delete, we use external events to drive the loop now
        void start();
        void stop();

        void onControllerDBFileChanged( QString controllerDBFile );

    signals:
        void deviceConnected( Joystick *joystick );
        void deviceRemoved( int which );

    private:
        void initSDL();
        void quitSDL();


        QTimer sdlPollTimer;
        int numOfDevices;
        QMutex sdlEventMutex;

        bool forceEventsHandling;
        bool mInitialized;

        // The InputManager is in charge of deleting these devices.
        // The InputManager gains access to these devices by the
        // deviceConnected( Joystick * ) signal.

        // Also for this list, make use of the 'which' index, for
        // proper insertions and retrievals.
        QList<Joystick *> sdlDeviceList;
        QHash<int, int> deviceLocationMap;
};

