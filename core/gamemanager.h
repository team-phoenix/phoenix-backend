#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "backendcommon.h"

#include "gamesession.h"
#include "logging.h"

/*
 * GameManager is a QML type that manages the execution of a game session.
 *
 * Internally, GameManager is just a QML wrapper for the underlying GameSession. It will provide a thread for GameSession
 * to operate under, and exposes a set of properties and methods for use by QML code, on the QML thread.
 *
 * From the perspective of QML, the interface exposed is similar to that of a game console. See docs for state property
 * for more info on how to run and manage the game session.
 */

class GameManager : public QObject {
        Q_OBJECT

        // Properties

        // Current error
        Q_PROPERTY( GameSession::Error error READ getError NOTIFY signalErrorChanged )

        // Master mute toggle, true iff volume is 0.0
        Q_PROPERTY( bool muted MEMBER muted NOTIFY signalMutedChanged )

        // Holds whether it's possible to pause this game session
        Q_PROPERTY( bool pausable READ getPausable NOTIFY signalPausableChanged )

        // playbackRate : real
        // Default: 1.0
        // Nonzero when playing, 0.0 when paused
        // Positive is forward playback, negative is backward playback (limited to size and granularity of rewind buffer)
        // Requested speed is not guarantied to be possible, and once rewind buffer empties game will be paused
        // It is an error to give a negative value when rewindable is false
        Q_PROPERTY( qreal playbackRate MEMBER playbackRate NOTIFY signalPlaybackRateChanged )

        // Holds whether it's possible to soft-reset this game session
        Q_PROPERTY( bool resettable READ getResettable NOTIFY signalResettableChanged )

        // Holds whether it's possible to rewind gameplay this game session
        Q_PROPERTY( bool rewindable READ getRewindable NOTIFY signalRewindableChanged )

        // Information about the game being played
        // The dictionary must contain the following:
        //     "type" : string -- The subsystem to be used in this session (mandatory)
        //
        //     Mandatory if "type" is set to "libretro":
        //     "core" : string -- Absolute path to a libretro core
        //     "game" : string -- Absolute path to a libretro-compatable game file
        Q_PROPERTY( QVariantMap source MEMBER source NOTIFY signalSourceChanged )

        // Current state
        // Note that this is not exactly the same as the underlying GameSession's state
        Q_PROPERTY( GameSession::State state READ getState NOTIFY signalStateChanged )

        // Reference to the VideoOutput object
        Q_PROPERTY( VideoOutput *videoOutput MEMBER videoOutput NOTIFY signalVideoOutputChanged )

        // Master volume, range: [0.0, 1.0]
        Q_PROPERTY( qreal volume MEMBER volume NOTIFY signalVolumeChanged )

    public:
        explicit GameManager( QObject *parent = 0 );

        // Attempt to load the given game (see source property)
        // Must be in STOPPED state
        void load();

        // Standard playback controls

        // Must be in PAUSED state
        void play();

        // Must be in PLAYING state, and the game must be pausable
        void pause();

        // Must be in either PAUSED or PLAYING state
        void stop();

        // Reset underlying game session, if possible
        // Must be in either PAUSED or PLAYING state, and the game must be resettable (see property)
        void reset();

    signals:
        // Property change notifiers
        void signalErrorChanged( GameSession::Error error );
        void signalMutedChanged( bool muted );
        void signalPausableChanged( bool pausable );
        void signalPlaybackRateChanged( bool playbackRate );
        void signalResettableChanged( bool resettable );
        void signalRewindableChanged( bool rewindable );
        void signalSourceChanged( QVariantMap source );
        void signalStateChanged( GameSession::State state );
        void signalVideoOutputChanged( VideoOutput *videoOutput );
        void signalVolumeChanged( qreal volume );

        // Messages to GameSession
        void signalLoadLibretro( QString core, QString game );
        void signalShutdown();

    public slots:
        // Messages from GameSession
        void slotError( GameSession::Error error );
        void slotInitFinished();
        void slotLoadFinished();
        void slotUnloadFinished();

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

        // Property getters
        GameSession::Error getError();
        bool getPausable();
        bool getResettable();
        bool getRewindable();
        GameSession::State getState();

        // GameSession and its thread
        QThread *gameThread;
        GameSession *gameSession;

        // Save the playback rate for later
        qreal savedPlaybackRate;

        // Helpers
        void notifyAllProperties();
        void setState( GameSession::State newState );
        void setError( GameSession::Error newError );
};

#endif // GAMEMANAGER_H
