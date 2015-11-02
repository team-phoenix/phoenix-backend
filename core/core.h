#ifndef CORE_H
#define CORE_H

#include "backendcommon.h"

#include "logging.h"

#include "producer.h"
#include "consumer.h"

/*
 * Superclass for all core plugins used by Phoenix. Defines a state machine similar to how a real game console operates
 * and presents an interface to manipulate this state machine. Games are generally loaded by calling setSource() then load().
 *
 * Minimal error checking is done in this class and in subclasses. It's expected that state changers such as pause() are
 * only called if Core is pausable. If not, the behavior is undefined and will probably cause crashes.
 *
 * Constructors of subclasses must set state to STOPPED and call allPropertiesChanged() once finished.
 *
 * Core is a producer of both audio and video data. At regular intervals, Core will send out signals containing pointers
 * to buffers. These pointers will internally be part of a circular buffer that will remain valid for the lifetime of Core.
 * To safely copy their contents, obtain a lock using either audioMutex or videoMutex.
 *
 * Core is also a consumer of input data.
 */

class Core : public QObject, public Producer, public Consumer {
        Q_OBJECT

    public:
        explicit Core( QObject *parent = 0 );
        virtual ~Core();

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
        Q_ENUM( State )
        Q_ENUMS( State )

    signals:
        // See producer.h
        PRODUCER_SIGNALS

        // Notifiers
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void sourceChanged( QStringMap source );
        void stateChanged( State state );
        void volumeChanged( qreal volume );

    public slots:
        // Information about the type of data to expect
        virtual void consumerFormat( ProducerFormat format ) = 0;

        // Must obtain a mutex to access the data. Only hold onto the mutex long enough to make a copy
        // Type can be one of the following: "audio", "video"
        virtual void consumerData( QString type, QMutex *mutex, void *data, size_t bytes ) = 0;

        // Setters
        virtual void setPlaybackSpeed( qreal playbackSpeed );
        virtual void setVolume( qreal volume );
        virtual void setSource( QStringMap source );

        // State changers
        virtual void load();
        virtual void play();
        virtual void pause();
        virtual void reset();
        virtual void stop();


    protected:
        // Property notifier helper
        void allPropertiesChanged();

        // Properties

        // Is this Core instance pausable? NOTE: "pausable" means whether or not you can *enter* State::PAUSED, not leave.
        // Core will ALWAYS enter State::PAUSED after State::LOADING regardless of this setting
        // Read-only
        bool pausable;

        // Multiplier of the system's native framerate, if any. If rewindable, it can be any real number. Otherwise, it must
        // be positive and nonzero
        // Read-write
        qreal playbackSpeed;

        // Is this Core instance resettable? If true, this usually means you can "soft reset" instead of fully resetting
        // the state machine by cycling through the deinit then init states
        // Read-only
        bool resettable;

        // Is this Core instance rewindable? If true, playbackSpeed may go to and below 0 to make the game move backwards
        // Read-only
        bool rewindable;

        // Subclass-defined info specific to this session (ex. Libretro: core, game, system and save paths)
        // Read-write
        QStringMap source;

        // Current state
        // Read-only
        State state;

        // Range: [0.0, 1.0]
        // Read-write
        qreal volume;

        // Setters
        virtual void setPausable( bool pausable );
        virtual void setResettable( bool resettable );
        virtual void setRewindable( bool rewindable );
        virtual void setState( State state );
        
};

#endif // CORE_H
