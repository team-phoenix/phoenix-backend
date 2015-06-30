#ifndef SDLEVENTLOOP_H
#define SDLEVENTLOOP_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QHash>
#include <SDL.h>

#include "joystick.h"

// The SDLEventLoop's job is to poll for button states,
// and to react the handle to newly connected, or disconnected, devices.

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

        void start();
        void stop();

    signals:

        void deviceConnected( Joystick *joystick );
        void deviceRemoved( int which );

    private:

        void initSDL();
        void quitSDL();

};

#endif // SDLEVENTLOOP_H
