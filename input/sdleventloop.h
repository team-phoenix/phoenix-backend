#ifndef SDLEVENTLOOP_H
#define SDLEVENTLOOP_H

#include "backendcommon.h"

#include "joystick.h"

// The SDLEventLoop's job is to poll for button states and to emit signals as devices are connected or disconnected.

class SDLEventLoop : public QObject {
        Q_OBJECT
        QTimer sdlPollTimer;
        int numOfDevices;
        QMutex sdlEventMutex;

        bool forceEventsHandling;

        // The InputManager is in charge of deleting these devices.
        // The InputManager gains access to these devices by the
        // deviceConnected( Joystick * ) signal.

        // Also for this list, make use of the 'which' index, for
        // proper insertions and retrievals.
        QList<Joystick *> sdlDeviceList;
        QHash<int, int> deviceLocationMap;

    public:
        explicit SDLEventLoop( QObject *parent = 0 );

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

};

#endif // SDLEVENTLOOP_H
