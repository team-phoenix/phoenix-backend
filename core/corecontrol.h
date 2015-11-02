#ifndef CORECONTROL_H
#define CORECONTROL_H

#include "backendcommon.h"

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
#include "looper.h"

/*
 * CoreControl is a QML type that manages the execution of an emulation session via an instance of Core.
 *
 * Internally, CoreControl is a QML proxy for the Core. It manages Core's lifecycle and connects it to consumers,
 * keeping them in a separate thread separate from the UI (except VideoOutput).
 * Writes via the set___() slot cause a ___changedProxy() signal to be emitted, which is then handled by a setter slot
 * in Core. Core emits a ___changed() signal of its own which we have connected to a set___Proxy() slot, which finally
 * assigns the proxied value and emits a ___changed() signal to notify anyone concerned.
 *
 * From the perspective of QML, the interface exposed is similar to that of a game console. See Core for more details.
 *
 */

class CoreControl : public QObject {
        Q_OBJECT

        // Properties
        Q_PROPERTY( VideoOutput *videoOutput MEMBER videoOutput )
        Q_PROPERTY( InputManager *inputManager MEMBER inputManager )

        // Core property proxy
        Q_PROPERTY( bool pausable READ getPausable NOTIFY pausableChanged )
        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )
        Q_PROPERTY( bool resettable READ getResettable NOTIFY resettableChanged )
        Q_PROPERTY( bool rewindable READ getRewindable NOTIFY rewindableChanged )
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )
        Q_PROPERTY( Core::State state READ getState NOTIFY stateChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )

    public:
        explicit CoreControl( QObject *parent = 0 );
        ~CoreControl();

        // Core property proxy
        // May be called from QML
        Q_INVOKABLE void load();
        Q_INVOKABLE void play();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void reset();

    signals:
        void beginLooper( double interval );

        // Core property proxy (Step 4: to anywhere)
        // These acknowledge that Core's state has been changed
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void sourceChanged( QVariantMap source );
        void stateChanged( Core::State state );
        void volumeChanged( qreal volume );

        // Core property proxy (Step 2: to Core)
        // These tell Core to change its state
        void playbackSpeedChangedProxy( qreal playbackSpeed );
        void sourceChangedProxy( QStringMap source );
        void volumeChangedProxy( qreal volume );

        // Core property proxy
        void loadProxy();
        void playProxy();
        void pauseProxy();
        void stopProxy();
        void resetProxy();


    public slots:
        // Core property proxy (Step 1: from anywhere)
        // These change Core's state ...once it gets around to it
        void setPlaybackSpeed( qreal playbackSpeed );
        void setSource( QVariantMap sourceQVariantMap );
        void setVolume( qreal volume );

        // Core property proxy (Step 3: from Core)
        // These tell us that Core has acknowledged our request and changed state
        void setPlaybackSpeedProxy( qreal playbackSpeed );
        void setSourceProxy( QStringMap sourceQStringMap );
        void setVolumeProxy( qreal volume );

    private:
        bool vsyncEnabled;

        // Controllers
        Looper *looper;
        QThread *looperThread;

        // Producers
        InputManager *inputManager;

        // Producer/consumer (Core)
        Core *core;
        QThread *coreThread;
        void loadLibretro();

        // Consumers
        AudioOutput *audioOutput;
        QThread *audioOutputThread;
        VideoOutput *videoOutput;

        // Core property proxy
        void notifyAllProperties();
        void connectCoreProxy();
        bool pausable;
        qreal playbackSpeed;
        bool resettable;
        bool rewindable;
        QStringMap source;
        Core::State state;
        qreal volume;
        bool getPausable() const;
        qreal getPlaybackSpeed() const;
        bool getResettable() const;
        bool getRewindable() const;
        QVariantMap getSource() const;
        Core::State getState() const;
        qreal getVolume() const;

};

#endif // CORECONTROL_H
