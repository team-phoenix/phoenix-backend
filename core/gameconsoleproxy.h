#pragma once

#include "backendcommon.h"
#include "controlhelper.h"

/*
 * GameConsoleProxy is a property proxy around its underlying GameConsole instance. Besides the many signals and slots
 * of the proxy system, all this class does is instantiate an instance of GameConsole and put it in its own thread.
 *
 * GameConsoleProxy is meant to be instantiated in your QML code.
 *
 * See GameConsole.h to see how to properly operate GameConsole.
 *
 * How the proxy system works:
 *   Writes via the GameConsoleProxy::set___() slot (step 1) cause a GameConsoleProxy::___changedProxy() (step 2) signal
 *   to be emitted, which is then handled by a GameConsole::set___() slot. GameConsole emits a GameConsole::___changed()
 *   signal (step 3) which we have connected to a GameConsoleProxy::set___Proxy() slot (step 4), which finally assigns
 *   the proxied value internally and emits a GameConsoleProxy___changed() signal to notify anyone concerned.
 *
 *   In total, per property, there are 2 GameConsoleProxy signals, 2 GameConsoleProxy slots, 1 GameConsole signal and
 *   1 GameConsole slot.
 *
 *   If the property is meant to be read-only, (step 1) and (step 2) are not needed.
 *
 * To add a new proxied property:
 *
 * - In GameConsole:
 *   - In GameConsole, set up a notifier signal and a setter slot along with a member
 *   - (Optional) If it's meant to go to some object like Core, in GameConsole set up a forwarder signal and make sure
 *     it gets registered inside GameConsole::connectCoreForwarder();
 *
 * - In GameConsoleProxy:
 *   - Set up an identical member here along with a getter, register with GameConsoleProxy::notifyAllProperties()
 *   - Set up 1 or 2 signals in GameConsoleProxy: A changed notifier (writable only) and a proxied change notifier
 *   - Set up 1 or 2 slots in GameConsoleProxy: A setter (writable only) and a proxied setter
 *   - If necessary, you can juggle two types for your property: one that's QML-friendly (ex. QVariantMap) and one
 *     that's more GameConsole/C++ friendly (ex. QStringMap)
 *     - Anything that touches QML gets the QML-friendly type (including the property, local member and getter)
 *     - Anything that goes to GameConsole gets the C++-friendly type (signals and slots that involve GameConsole)
 *     - Convert as needed
 *
 * To add a new forwarded method:
 *
 * - Add a slot to GameConsole
 * - Add a new public Q_INVOKABLE public method here, have it emit a forwarder signal, register in:
 *     GameConsoleProxy::connectGameConsoleProxy();
 * - (Optional) In GameConsole, set up a forwarder signal and register the forwarder inside :
 *     GameConsole::connectCoreForwarder();
 */

#include "videooutput.h"
#include "gamepadmanager.h"

#include <QQuickItem>
#include <QQmlParserStatus>

class QCoreApplication;
class GameConsole;
class QThread;

class GameConsoleProxy : public QObject, public QQmlParserStatus {
        Q_OBJECT
        Q_INTERFACES( QQmlParserStatus )

        // Settings

        // Producers and consumers that live in QML (and the QML thread)

        // Needed by: "libretro"
        Q_PROPERTY( VideoOutput *videoOutput READ getVideoOutput WRITE setVideoOutput NOTIFY videoOutputChanged )

        // Proxy
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )

        Q_PROPERTY( PlaybackState playbackState READ getPlaybackState NOTIFY playbackStateChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )
        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )

        Q_PROPERTY( bool vsync READ getVsync WRITE setVsync NOTIFY vsyncChanged )
        Q_PROPERTY( bool pausable READ getPausable NOTIFY pausableChanged )
        Q_PROPERTY( bool resettable READ getResettable NOTIFY resettableChanged )
        Q_PROPERTY( bool rewindable READ getRewindable NOTIFY rewindableChanged )

    public:
        explicit GameConsoleProxy( QObject *parent = nullptr );
        ~GameConsoleProxy() = default;

        enum class PlaybackState {
            Playing,
            Stopped,
            Paused,
            Loading,
            Unloading,
            Resetting,
        };
        Q_ENUM( PlaybackState )

        // May be called from QML
        Q_INVOKABLE void load();
        Q_INVOKABLE void play();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void reset();

        // QML engine listeners
        void componentComplete() override {}
        void classBegin() override;

        // QML Getters
        VideoOutput *getVideoOutput() const;

        qreal getPlaybackSpeed() const;
        PlaybackState getPlaybackState() const;

        qreal getVolume() const;

        QVariantMap getSource() const;
        bool getPausable() const;
        bool getResettable() const;
        bool getRewindable() const;
        bool getVsync() const;


        // QML Setters
        void setSource( QVariantMap source );
        void setVideoOutput( VideoOutput *videoOutput );
        void setPlaybackSpeed( qreal speed );
        void setVolume( qreal volume );
        void setVsync( bool vsync );
        void setRewindable( bool rewindable );
        void setResettable(bool resettable );
        void setPausable( bool pausable );

    signals:
        // FIXME: Delete these
        void gamepadAdded( const Gamepad *gamepad );
        void gamepadRemoved( const Gamepad *gamepad );

        // GameConsole property proxy (Step 4: to anywhere)
        // These acknowledge that GameConsole's state has been changed
        void videoOutputChanged();
        void pausableChanged();
        void playbackSpeedChanged();
        void resettableChanged();
        void rewindableChanged();
        void sourceChanged();
        void volumeChanged();
        void vsyncChanged();
        void playbackStateChanged();

    private:
        GameConsole *gameConsole;
        QThread *gameThread;

        // GameConsole property proxy
        VideoOutput *videoOutput;

        QVariantMap source;

        qreal playbackSpeed{ 1.0 };
        qreal volume{ 1.0 };

        bool vsync{ false };
        bool resettable{ false };
        bool rewindable{ false };
        bool pausable{ false };
        PlaybackState playbackState{ PlaybackState::Stopped };

        void setPlaybackState( PlaybackState state );

};

Q_DECLARE_METATYPE( const Gamepad * )
