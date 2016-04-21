#pragma once

#include "control.h"
#include "producer.h"
#include "gamepadmanager.h"
#include "pipelinenode.h"

#include <QUrl>
#include <QObject>
#include <QMap>
#include <QVariant>


/*
 * GameConsole is a control object that manages the execution of an emulation session via an instance of Core.
 * In other words, GameConsole is the glue that holds the other components together.
 *
 * GameConsole manages Core's lifecycle and connects it to consumers, which live in different threads.
 *
 * Externally, the interface exposed is similar to that of a game console. See Control (control.h) for more details.
 *
 * WARNING: Do not call load() until you have passed refernces to instances of the consumers and producers needed
 * for your particular Core subtype:
 *
 * "libretro": GamepadManager, VideoOutput
 *
 * WARNING: Ensure that all control objects (such as Looper, LibretroCore and this object) live in the same thread. It
 * is critical that all control signals are synchronous.
 *
 * Only the following moves are legal:
 * STATE: methodsAllowed()
 *
 * STOPPED: load()
 * LOADING:
 * PAUSED: load(), play(), stop()
 * PLAYING: load(), pause(), stop()
 * UNLOADING:
 *
 * Note that while GameConsole and Core share the same thread, methods valid for PAUSED may be called during LOADING
 * and methods valid for STOPPED may be called during UNLOADING as these are transitional states.
 *
 */


class VideoOutput;
class AudioOutput;
class Gamepad;
class Looper;
class Core;

class GameConsole : public QObject, public Control {
        Q_OBJECT
        PHX_PIPELINE_INTERFACE( GameConsole )

    public:
        explicit GameConsole( QObject *parent = 0 );
        ~GameConsole() = default;

    signals:
        CONTROL_SIGNALS

        // Notifiers
        void gamepadAdded( const Gamepad * );
        void gamepadRemoved( const Gamepad * );

        void videoOutputChanged( VideoOutput *videoOutput );
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void stateChanged( Control::State currentState );
        void volumeChanged( qreal volume );
        void vsyncChanged( bool vsync );

        // Setter forwarders (to Core)
        void setPlaybackSpeedForwarder( qreal playbackSpeed );
        void setVolumeForwarder( qreal volume );

        // Special

        // Necessary in certain scenarios to emit this ourselves (like getting a chain reaction going in VideoOutput)
        void libretroCoreDoFrame( qint64 timestamp );

        // Inform any consumer or producer about the rate that drives the production of frames, which may be different
        // than the native framerate
        void libretroSetFramerate( qreal hostFPS );

    public slots:
        // We cannot use the destructor to clean up, we can't emit signals to anything anymore
        // Use this instead to do all cleanup

        void dataIn( PipelineNode::DataReason t_reason
                     , QMutex *t_mutex
                     , void *t_data
                     , size_t t_bytes
                     , qint64 t_timeStamp );

        void shutdown();

        void setSrc( QVariant _src );

        // Setters (from anywhere)
        void setVideoOutput( VideoOutput *videoOutput );
        void setPlaybackSpeed( qreal playbackSpeed );
        void setVolume( qreal volume );
        void setVsync( bool vsync );

        // State changers
        void load();
        void play();
        void pause();
        void stop();
        void reset();

    private:
        QString m_coreSrc;
        QString m_gameSrc;
        QVariant m_src;

        // Thread management

        // Holds all the children for each thread in use
        // WARNING: You must register all objects created in your initXXXX() function to *one* of these two lists if not
        // a member of the QML thread!
        QMap<QThread *, QList<QObject *>> threadChildren;
        QList<QObject *> gameThreadChildren;

        // Register any pointers to memory you allocate in your init function here so they may be zeroed on shutdown
        QList<QObject **> pointersToClear;
        void zeroPointers();

        // Delete all threads in threadChildren (children are implicitly deleted)
        void deleteThreads();

        // Delete all children in gameThreadChildren directly (these children all live in the current thread)
        void deleteGameThreadChildren();

        // Run all of the above, in order
        void cleanup();

        // Controllers

        Looper *looper{ nullptr };

        // Producers
        GamepadManager m_gamepadManager;

        // Core (and core loaders)

        Core *core{ nullptr };
        void connectCoreForwarder();
        void trackCoreStateChanges();

        // Consumers

        AudioOutput *audioOutput{ nullptr };
        QThread *audioOutputThread{ nullptr };
        VideoOutput *videoOutput{ nullptr };
        bool vsync{ false };

        // Misc

        // A list of connections we've made this session. Make sure you've registered ALL connections with this list!
        // You can also make an ad-hoc lambda slot that disconnects itself on use (single-shot)
        QList<QMetaObject::Connection> connectionList;
        void disconnectConnections();

        // Core loaders

        void initLibretroCore();


};
