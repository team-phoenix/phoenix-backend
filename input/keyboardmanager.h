#pragma once

#include <QMutex>
#include <QObject>

#include "SDL.h"
#include "SDL_gamecontroller.h"

#include "node.h"
#include "keyboardstate.h"

class KeyboardMouseListener;

/*
 * KeyboardManager is a Node whose job is to produce input data from the keyboard. It does this by listening to its
 * two slots, keyPressed() and keyReleased(), which are connected to a KeyboardMouseListener living on the main thread.
 */

class KeyboardManager : public Node {
        Q_OBJECT

    public:
        KeyboardManager() = default;
        void connectKeyboardInput( KeyboardMouseListener *keyboardInput );

    signals:

    public slots:
        void keyPressed( int key );
        void keyReleased( int key );
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

    private:
        // Holds the keyboard state as it's being built
        KeyboardState keyboard;

        // Circular buffer of keyboard states
        KeyboardState keyboardBuffer[ 100 ];
        int keyboardBufferIndex { 0 };

        // Ensure reads/writes to keyboardBuffer are atomic
        QMutex mutex;

        // Helpers
        void insertState( int key, bool state );
};
