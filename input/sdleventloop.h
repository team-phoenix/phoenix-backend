#pragma once

#include "joystickevent.h"
#include "SDL_gamecontroller.h"
#include "gamepad.h"

#include <QObject>
#include <QHash>



class Gamepad;
class Joystick;

// The SDLEventLoop's job is to poll for button states and to emit signals as devices are connected or disconnected.

class SDLEventLoop : public QObject {
        Q_OBJECT
    public:
        explicit SDLEventLoop( QObject *parent = nullptr );
        ~SDLEventLoop();

    public slots:
        void poll( qint64 timeStamp = 0 );
        void onControllerDBFileChanged( QString controllerDBFile );

    signals:
        void gamepadAdded( const Gamepad * );
        void gamepadRemoved( const Gamepad *);

    private:

        QHash<qint32, Gamepad *> m_gamepadsMap;


        void init( const QByteArray &mapData );
        void deinit();

};
