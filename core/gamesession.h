#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "backendcommon.h"

#include "libretro.h"
#include "libretrocore.h"

// Consumers
#include "audiooutput.h"
#include "videooutput.h"

/*
 * GameSession encapsulates a game session. A QML interface to this class is available through GameManager.
 * It lives in the same thread as the cores, unlike GameManager.
 * By default, it will output audio to the system's default audio device and video to the given VideoOutput instance
 */

class GameSession : public QObject {
        Q_OBJECT

    public:
        explicit GameSession( QObject *parent = 0 );

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
            ERRORED
        };
        Q_ENUMS( State )

        enum Error {
            NOERR = 0,
            UNKNOWNERROR = 1
        };
        Q_ENUMS( Error )

    signals:
        // Implicit state change signals, meant to go to GameManager
        void signalError( GameSession::Error error );
        void signalInitFinished();
        void signalLoadFinished();
        void signalUnloadFinished();

        // Signals for libretro cores
        void loadLibretroCore( QString core );
        void loadLibretroGame( QString game );

    public slots:
        void loadLibretro( QString core, QString game );

    private:
        // Properties
        GameSession::Error error;
        bool muted;
        bool pausable;
        qreal playbackRate;
        bool resettable;
        bool rewindable;
        QVariantMap source;
        GameSession::State state;
        VideoOutput *videoOutput;
        qreal volume;

        // Cores
        LibretroCore libretroCore;

};

#endif // GAMESESSION_H
