#ifndef CORECOMMON_H
#define CORECOMMON_H

#include <QtCore>

namespace phx {
    // States
    enum State {
        // Still in the constructor, probably allocating memory or something
        INIT = 0,

        // Nothing's loaded. Give GameManager a game to play then call load()
        STOPPED,

        // Game is currently loading
        LOADING,

        // Game fully loaded, not running. Call play() to resume game or stop() to stop playing
        PAUSED,

        // Game fully loaded, running. Call pause() to pause game or stop() to stop playing
        PLAYING,

        // Game is shutting down. It could be doing stuff like saving game data back to disk or freeing memory
        UNLOADING,

        // Something went wrong. Check the error property and handle accordingly. stop() the session once
        // you've handled the situation. GameManager will clean up memory and should be ready to launch
        // again.
        ERROR
    };
    Q_ENUMS( State )
}
#endif // CORECOMMON_H

