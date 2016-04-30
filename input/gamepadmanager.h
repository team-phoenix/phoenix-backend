#pragma once

#include <QObject>
#include <QMutex>
#include <QHash>

#include "gamepad.h"
#include "node.h"

#include "SDL.h"
#include "SDL_gamecontroller.h"

class GamepadManager : public Node {
        Q_OBJECT

    public:
        explicit GamepadManager( Node *parent = 0 );

    signals:

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

    private:
        QMutex mutex;

        // A list of stored button states, indexed by instanceID
        QHash<int, Gamepad> gamepads;

        // Handles to the underlying game controller API handle, indexed by instanceID
        QHash<int, SDL_GameController *> gamepadHandles;

        // A circular buffer that holds gamepad state updates
        // A value of 100 should be sufficient for most purposes
        Gamepad gamepadBuffer[ 100 ];
        int gamepadBufferIndex{ 0 };
};
