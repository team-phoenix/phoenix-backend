#ifndef CORE_H
#define CORE_H

#include "backendcommon.h"

#include "controllable.h"
#include "producer.h"
#include "consumer.h"

#include <QUrl>
#include <QDebug>

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

class Core : public QObject, public Producer, public Consumer, public Controllable {
        Q_OBJECT

    public:
        explicit Core( QObject *parent = 0 );
        virtual ~Core() = default;

    signals:
        PRODUCER_SIGNALS

        // Notifiers
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void stateChanged( Control::State currentState );
        void volumeChanged( qreal volume );

    public slots:
        CONSUMER_SLOTS_ABSTRACT
        virtual void setState( Control::State state ) override;

        // Setters
        virtual void setPlaybackSpeed( qreal playbackSpeed );
        virtual void setVolume( qreal volume );

        virtual void setSrc( QVariant _src ) = 0;

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
        bool pausable{ false };

        // Multiplier of the system's native framerate, if any. If rewindable, it can be any real number. Otherwise, it must
        // be positive and nonzero
        // Read-write
        qreal playbackSpeed{ 1.0 };

        // Is this Core instance resettable? If true, this usually means you can "soft reset" instead of fully resetting
        // the state machine by cycling through the deinit then init states
        // Read-only
        bool resettable{ false };

        // Is this Core instance rewindable? If true, playbackSpeed may go to and below 0 to make the game move backwards
        // Read-only
        bool rewindable{ false };

        // Subclass-defined info specific to this session (ex. Libretro: core, game, system and save paths)
        // Read-write


        // Range: [0.0, 1.0]
        // Read-write
        qreal volume{ 1.0 };

        // Setters
        virtual void setPausable( bool pausable );
        virtual void setResettable( bool resettable );
        virtual void setRewindable( bool rewindable );
        
};

#endif // CORE_H
