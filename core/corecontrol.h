#ifndef CORECONTROL_H
#define CORECONTROL_H

#include "backendcommon.h"

#include "control.h"
#include "controllable.h"

// Timers that drive frame production (controllers)
#include "looper.h"

// Producers
#include "producer.h"
#include "inputmanager.h"
#include "core.h"
#include "libretrocore.h"

// Consumers
#include "consumer.h"
#include "audiooutput.h"
#include "videooutput.h"

// Misc
#include "logging.h"

/*
 * CoreControl is a control object that manages the execution of an emulation session via an instance of Core.
 * In other words, CoreControl is the glue that holds the other components together.
 *
 * CoreControl manages Core's lifecycle and connects it to consumers, which live in different threads.
 *
 * Externally, the interface exposed is similar to that of a game console. See Control (control.h) for more details.
 *
 * WARNING: Do not call load() until you have passed refernces to instances of the consumers and producers needed
 * for your particular Core subtype:
 *
 * "libretro": InputManager, VideoOutput
 *
 * WARNING: Ensure that all control objects (such as Looper, LibretroCore and this object) live in the same thread. It
 * is critical that all control signals are synchronous.
 *
 */

class CoreControl : public QObject, public Control {
        Q_OBJECT

    public:
        explicit CoreControl( QObject *parent = 0 );
        ~CoreControl();

    signals:
        CONTROL_SIGNALS

        // Notifiers
        void videoOutputChanged( VideoOutput *videoOutput );
        void inputManagerChanged( InputManager *inputManager );
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void sourceChanged( QStringMap source );
        void stateChanged( Control::State currentState );
        void volumeChanged( qreal volume );
        void vsyncChanged( bool vsync );

        // Setter forwarders (to Core)
        void setPlaybackSpeedForwarder( qreal playbackSpeed );
        void setSourceForwarder( QStringMap source );
        void setVolumeForwarder( qreal volume );

        // Method forwarders (to Core)
        void loadForwarder();
        void playForwarder();
        void pauseForwarder();
        void stopForwarder();
        void resetForwarder();

        // Special

        // Necessary in certain scenarios to emit this ourselves (like getting a chain reaction going in VideoOutput)
        void libretroCoreDoFrame();

        // Inform any consumer or producer about the rate that drives the production of frames, which may be different
        // than the native framerate
        void libretroSetFramerate( qreal hostFPS );

    public slots:
        // We cannot use the destructor to clean up, we can't emit signals to anything anymore
        // Use this instead to do all cleanup
        void shutdown();

        // Setters (from anywhere)
        void setVideoOutput( VideoOutput *videoOutput );
        void setInputManager( InputManager *inputManager );
        void setPlaybackSpeed( qreal playbackSpeed );
        void setSource( QStringMap source );
        void setVolume( qreal volume );
        void setVsync( bool vsync );

        // State changers
        void load() override;
        void play() override;
        void pause() override;
        void stop() override;
        void reset() override;

    private:
        // Thread management

        // Holds all the children for each thread in use
        // WARNING: You must register all objects created in your initXXXX() function to *one* of these two lists if not
        // a member of the QML thread!
        QMap<QThread *, QList<QObject *>> threadChildren;
        QList<QObject *> gameThreadChildren;

        // Delete all threads in threadChildren (children are implicitly deleted)
        void deleteThreads();

        // Delete all children in gameThreadChildren directly (these children all live in the current thread)
        void deleteGameThreadChildren();

        // Run all of the above, in order
        void cleanup();

        // Controllers

        Looper *looper;

        // Producers

        InputManager *inputManager;

        // Core (and core loaders)

        Core *core;
        void connectCoreForwarder();
        void trackCoreStateChanges();

        // Consumers

        AudioOutput *audioOutput;
        QThread *audioOutputThread;
        VideoOutput *videoOutput;
        bool vsync;

        // Misc

        // A list of *reusable* ad-hoc lambda slots we've created for this session
        // You can also make an ad-hoc lambda slot that disconnects itself on use (single-shot)
        QList<QMetaObject::Connection> connectionList;
        void disconnectConnections();

        // Core loaders

        void initLibretroCore();
};

#endif // CORECONTROL_H
