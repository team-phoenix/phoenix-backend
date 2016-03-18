#ifndef CORECONTROLPROXY_H
#define CORECONTROLPROXY_H

#include "backendcommon.h"

#include "corecontrol.h"
#include "controlhelper.h"

/*
 * CoreControlProxy is a property proxy around its underlying CoreControl instance. Besides the many signals and slots
 * of the proxy system, all this class does is instantiate an instance of CoreControl and put it in its own thread.
 *
 * CoreControlProxy is meant to be instantiated in your QML code.
 *
 * See corecontrol.h to see how to properly operate CoreControl.
 *
 * How the proxy system works:
 *   Writes via the CoreControlProxy::set___() slot (step 1) cause a CoreControlProxy::___changedProxy() (step 2) signal
 *   to be emitted, which is then handled by a CoreControl::set___() slot. CoreControl emits a CoreControl::___changed()
 *   signal (step 3) which we have connected to a CoreControlProxy::set___Proxy() slot (step 4), which finally assigns
 *   the proxied value internally and emits a CoreControlProxy___changed() signal to notify anyone concerned.
 *
 *   In total, per property, there are 2 CoreControlProxy signals, 2 CoreControlProxy slots, 1 CoreControl signal and
 *   1 CoreControl slot.
 *
 *   If the property is meant to be read-only, (step 1) and (step 2) are not needed.
 *
 * To add a new proxied property:
 *
 * - In CoreControl:
 *   - In CoreControl, set up a notifier signal and a setter slot along with a member
 *   - (Optional) If it's meant to go to some object like Core, in CoreControl set up a forwarder signal and make sure
 *     it gets registered inside CoreControl::connectCoreForwarder();
 *
 * - In CoreControlProxy:
 *   - Set up an identical member here along with a getter, register with CoreControlProxy::notifyAllProperties()
 *   - Set up 1 or 2 signals in CoreControlProxy: A changed notifier (writable only) and a proxied change notifier
 *   - Set up 1 or 2 slots in CoreControlProxy: A setter (writable only) and a proxied setter
 *   - If necessary, you can juggle two types for your property: one that's QML-friendly (ex. QVariantMap) and one
 *     that's more CoreControl/C++ friendly (ex. QStringMap)
 *     - Anything that touches QML gets the QML-friendly type (including the property, local member and getter)
 *     - Anything that goes to CoreControl gets the C++-friendly type (signals and slots that involve CoreControl)
 *     - Convert as needed
 *
 * To add a new forwarded method:
 *
 * - Add a slot to CoreControl
 * - Add a new public Q_INVOKABLE public method here, have it emit a forwarder signal, register in:
 *     CoreControlProxy::connectCoreControlProxy();
 * - (Optional) In CoreControl, set up a forwarder signal and register the forwarder inside :
 *     CoreControl::connectCoreForwarder();
 */

class CoreControlProxy : public QObject {
        Q_OBJECT

        // Settings

        // Producers and consumers that live in QML (and the QML thread)

        // Needed by: "libretro"
        Q_PROPERTY( QObject *videoOutput READ getVideoOutput WRITE setVideoOutput NOTIFY videoOutputChanged )

        // Needed by: "libretro"
        Q_PROPERTY( QObject *inputManager READ getInputManager WRITE setInputManager NOTIFY inputManagerChanged )

        // Proxy
        Q_PROPERTY( bool pausable READ getPausable NOTIFY pausableChanged )
        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )
        Q_PROPERTY( bool resettable READ getResettable NOTIFY resettableChanged )
        Q_PROPERTY( bool rewindable READ getRewindable NOTIFY rewindableChanged )
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )
        Q_PROPERTY( ControlHelper::State state READ getState NOTIFY stateChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )
        Q_PROPERTY( bool vsync READ getVsync WRITE setVsync NOTIFY vsyncChanged )

    public:
        explicit CoreControlProxy( QObject *parent = 0 );
        ~CoreControlProxy();

        // May be called from QML
        Q_INVOKABLE void load();
        Q_INVOKABLE void play();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void reset();

    signals:
        // Tell CoreControl to clean up
        void shutdown();

        // CoreControl property proxy (Step 4: to anywhere)
        // These acknowledge that CoreControl's state has been changed
        void videoOutputChanged( VideoOutput *videoOutput );
        void inputManagerChanged( InputManager *inputManager );
        void pausableChanged( bool pausable );
        void playbackSpeedChanged( qreal playbackSpeed );
        void resettableChanged( bool resettable );
        void rewindableChanged( bool rewindable );
        void sourceChanged( QVariantMap source );
        void stateChanged( ControlHelper::State state );
        void volumeChanged( qreal volume );
        void vsyncChanged( bool vsync );

        // CoreControl property proxy (Step 2: to CoreControl)
        // These tell CoreControl to change its state
        void videoOutputChangedProxy( VideoOutput *videoOutput );
        void inputManagerChangedProxy( InputManager *inputManager );
        void playbackSpeedChangedProxy( qreal playbackSpeed );
        void sourceChangedProxy( QStringMap source );
        void volumeChangedProxy( qreal volume );
        void vsyncChangedProxy( bool vsync );

        // CoreControl method forwarder
        void loadForwarder();
        void playForwarder();
        void pauseForwarder();
        void stopForwarder();
        void resetForwarder();

    private slots:
        // CoreControl property proxy (Step 1: from anywhere)
        // These change CoreControl's state ...once it gets around to it
        void setVideoOutput( QObject *videoOutput );
        void setInputManager( QObject *inputManager );
        void setPlaybackSpeed( qreal playbackSpeed );
        void setSource( QVariantMap sourceQVariantMap );
        void setVolume( qreal volume );
        void setVsync( bool vsync );

        // CoreControl property proxy (Step 3: from CoreControl)
        // These tell us that CoreControl has acknowledged our request and changed state
        void setVideoOutputProxy( VideoOutput *videoOutput );
        void setInputManagerProxy( InputManager *inputManager );
        void setPausableProxy( bool pausable );
        void setPlaybackSpeedProxy( qreal playbackSpeed );
        void setResettableProxy( bool resettable );
        void setRewindableProxy( bool rewindable );
        void setSourceProxy( QStringMap sourceQStringMap );
        void setStateProxy( Control::State state );
        void setVolumeProxy( qreal volume );
        void setVsyncProxy( bool vsync );

    private:
        CoreControl *coreControl;
        QThread *gameThread;

        // CoreControl property proxy
        void notifyAllProperties();
        void connectCoreControlProxy();
        VideoOutput *videoOutput;
        InputManager *inputManager;
        bool pausable;
        qreal playbackSpeed;
        bool resettable;
        bool rewindable;
        QStringMap source;
        ControlHelper::State state;
        qreal volume;
        bool vsync;

        VideoOutput *getVideoOutput() const;
        InputManager *getInputManager() const;
        bool getPausable() const;
        qreal getPlaybackSpeed() const;
        bool getResettable() const;
        bool getRewindable() const;
        QVariantMap getSource() const;
        ControlHelper::State getState() const;
        qreal getVolume() const;
        bool getVsync() const;
};

#endif // CORECONTROLPROXY_H
