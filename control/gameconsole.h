#pragma once

#include <QObject>
#include <QThread>
#include <QVariant>

#include "node.h"

// Nodes
#include "audiooutput.h"
#include "gamepadmanager.h"
#include "globalgamepad.h"
#include "libretrocore.h"
#include "metaoutput.h"
#include "microtimer.h"
#include "remapper.h"
#include "videooutput.h"

#include "controlhelper.h"

class GameConsole : public Node {
        Q_OBJECT

        Q_PROPERTY( GlobalGamepad *globalGamepad MEMBER globalGamepad )
        Q_PROPERTY( MetaOutput *metaOutput MEMBER metaOutput )
        Q_PROPERTY( VideoOutput *videoOutput MEMBER videoOutput )

        Q_PROPERTY( bool pausable READ getPausable NOTIFY pausableChanged )
        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )
        Q_PROPERTY( bool resettable READ getResettable NOTIFY resettableChanged )
        Q_PROPERTY( bool rewindable READ getRewindable NOTIFY rewindableChanged )
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )
        Q_PROPERTY( ControlHelper::State state READ getState NOTIFY stateChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )
        Q_PROPERTY( bool vsync READ getVsync WRITE setVsync NOTIFY vsyncChanged )

    public:
        explicit GameConsole( Node *parent = 0 );
        ~GameConsole();

    signals:
        // Properties
        void pausableChanged();
        void playbackSpeedChanged();
        void resettableChanged();
        void rewindableChanged();
        void sourceChanged();
        void stateChanged();
        void volumeChanged();
        void vsyncChanged();

    public slots:
        void load();
        void play();
        void pause();
        void stop();
        void reset();

    private:
        // API-specific loaders
        void loadLibretro();

        // Emulation thread
        QThread *gameThread;

        // Pipeline nodes owned by this class (game thread)
        AudioOutput *audioOutput{ nullptr };
        GamepadManager *gamepadManager{ nullptr };
        LibretroCore *libretroCore{ nullptr };
        MicroTimer *microTimer{ nullptr };
        Remapper *remapper{ nullptr };

        // Pipeline nodes owned by the QML engine (main thread)
        // Must be given to us via properties
        GlobalGamepad *globalGamepad{ nullptr };
        MetaOutput *metaOutput{ nullptr };
        VideoOutput *videoOutput{ nullptr };

        // Properties
        bool pausable;
        bool getPausable();
        qreal playbackSpeed;
        qreal getPlaybackSpeed();
        void setPlaybackSpeed( qreal playbackSpeed );
        bool resettable;
        bool getResettable();
        bool rewindable;
        bool getRewindable();
        QVariantMap source;
        QVariantMap getSource();
        void setSource( QVariantMap source );
        ControlHelper::State state;
        ControlHelper::State getState();
        qreal volume;
        qreal getVolume();
        void setVolume( qreal volume );
        bool vsync;
        bool getVsync();
        void setVsync( bool vsync );
};
