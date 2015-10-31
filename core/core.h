#ifndef CORE_H
#define CORE_H

#include "backendcommon.h"

#include "logging.h"

// Consumers
#include "audiooutput.h"
#include "videooutput.h"

/*
 * Superclass for all core plugins used by Phoenix. Defines a state machine similar to how a real game console operates
 * and presents an interface to manipulate this state machine. Games are generally loaded by calling setSource() then load().
 *
 * Minimal error checking is done in this class and in subclasses. It's expected that state changers such as pause() are
 * only called if Core is pausable.
 *
 * Constructors of subclasses must call stateChanged( State::STOPPED ) once the constructor is finished.
 *
 * Core is a producer of both audio and video data. At regular intervals, Core will send out signals containing pointers
 * to buffers. These pointers will internally be part of a circular buffer that will remain valid for the lifetime of Core.
 * To safely copy their contents, obtain a lock using either audioMutex or videoMutex.
 */

class Core : public QObject {
        Q_OBJECT

    public:
        explicit Core( QObject *parent = 0 );

        // States
        enum State {
            // Still in the constructor, probably allocating memory or something
            INIT = 0,

            // Nothing's loaded. Give Core a game to play with setSource() then do load()
            STOPPED,

            // Game is currently loading
            LOADING,

            // Game fully loaded, not running. Call play() to resume game or stop() to stop playing
            PAUSED,

            // Game fully loaded, running. Call pause() to pause game or stop() to stop playing
            PLAYING,

            // Game is shutting down. It could be doing stuff like saving game data back to disk or freeing memory
            UNLOADING
        };
        Q_ENUMS( State )

        // Only hold onto these mutexes long enough to make a copy
        QMutex audioMutex;
        QMutex videoMutex;

    signals:
        // Notifiers
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void sourceChanged( QMap<QString, QString> source );
        void stateChanged( State state );
        void volumeChanged( qreal volume );

        // Consumer data. Pointers will be valid for the lifetime of Core. A circular buffer pool is recommended to
        // accomplish this.
        void audioData( void *data, long bytes );
        void videoData( void *data, long bytes );

    public slots:
        // Setters
        void setPausable( bool pausable );
        void setPlaybackSpeed( qreal playbackSpeed );
        void setResettable( bool resettable );
        void setRewindable( bool rewindable );
        void setSource( QMap<QString, QString> source );
        void setVolume( qreal volume );

        // State changers
        void load();
        void play();
        void pause();
        void reset();
        void stop();

        void setState( State state );

    protected:

        // Properties

        // Is this Core instance pausable? NOTE: "pausable" means whether or not you can *enter* State::PAUSED, not leave.
        // Core will ALWAYS enter State::PAUSED after State::LOADING regardless of this setting
        bool pausable;

        // Playback speed. 1.0 is considered "normal" speed. If rewindable, it can be any real number. Otherwise, it must
        // be positive and nonzero
        qreal playbackSpeed;

        // Is this Core instance resettable? If true, this usually means you can "soft reset" instead of fully resetting
        // the state machine by cycling through the deinit then init states
        bool resettable;

        // Is this Core instance rewindable? If true, playbackSpeed may go to and below 0 to make the game move backwards
        bool rewindable;

        // Subclass-defined info specific to this session (ex. Libretro: core, game, system and save paths)
        QMap<QString, QString> source;

        // Current state
        State state;

        // Range: [0.0, 1.0]
        qreal volume;
};

#endif // CORE_H
