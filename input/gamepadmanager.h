#pragma once

#include <QObject>
#include <QMutex>
#include <QHash>

#include "gamepadstate.h"
#include "node.h"

#include "SDL.h"
#include "SDL_gamecontroller.h"

/*
 * GamepadManager is a Node whose job is to produce input data from physical controllers connected to the system running
 * Phoenix. The polling of controllers for their existence and input data is driven by heartbeats from this node's parent.
 */

class GamepadManager : public Node {
        Q_OBJECT

    public:
        explicit GamepadManager( Node *parent = nullptr );

    signals:

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

    private:
        QMutex mutex;

        // A list of stored button states, indexed by instanceID
        QHash<int, GamepadState> gamepads;

        // Handles to the underlying game controller API handle, indexed by instanceID
        QHash<int, SDL_GameController *> gamepadHandles;

        // A circular buffer that holds gamepad state updates
        // A value of 100 should be sufficient for most purposes
        GamepadState gamepadBuffer[ 100 ];
        int gamepadBufferIndex{ 0 };

        QString userDataPath;
};
