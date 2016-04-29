#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <QThread>
#include <QVariant>

#include "node.h"

// Nodes
#include "audiooutput.h"
#include "gamepadmanager.h"
#include "globalgamepad.h"
#include "libretrocore.h"
#include "controloutput.h"
#include "microtimer.h"
#include "phoenixwindow.h"
#include "phoenixwindownode.h"
#include "remapper.h"
#include "videooutput.h"
#include "videooutputnode.h"

#include "controlhelper.h"

class GameConsole : public Node, public QQmlParserStatus {
        Q_OBJECT
        Q_INTERFACES( QQmlParserStatus )

        Q_PROPERTY( ControlOutput *controlOutput MEMBER controlOutput NOTIFY controlOutputChanged )
        Q_PROPERTY( GlobalGamepad *globalGamepad MEMBER globalGamepad NOTIFY globalGamepadChanged )
        Q_PROPERTY( PhoenixWindowNode *phoenixWindow MEMBER phoenixWindow NOTIFY phoenixWindowChanged )
        Q_PROPERTY( VideoOutputNode *videoOutput MEMBER videoOutput NOTIFY videoOutputChanged )

        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )
        Q_PROPERTY( bool vsync READ getVsync WRITE setVsync NOTIFY vsyncChanged )

    public:
        explicit GameConsole( Node *parent = 0 );
        ~GameConsole();

        void classBegin() override;
        void componentComplete() override;

    public slots:
        void load();
        void play();
        void pause();
        void stop();
        void reset();
        void unload();

    private:
        // API-specific loaders
        void loadLibretro();
        void unloadLibretro();

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
        ControlOutput *controlOutput{ nullptr };
        GlobalGamepad *globalGamepad{ nullptr };
        PhoenixWindowNode *phoenixWindow{ nullptr };
        VideoOutputNode *videoOutput{ nullptr };

        // Keeps track of session connections so they may be disconnected once emulation ends
        QList<QMetaObject::Connection> sessionConnections;

        // Return true if all global pipeline members from QML are set
        bool globalPipelineReady();

        // Return true if a core is loaded and its dynamic pipeline is hooked to the global one
        bool dynamicPipelineReady();

        // Used to stop the game thread on app quit
        bool quitFlag{ false };

        // QVariantMap that holds property changes that were set before the global pipeline loaded
        QVariantMap pendingPropertyChanges;
        void applyPendingPropertyChanges();

    private: // Property getters/setters
        qreal playbackSpeed{ 1.0 };
        qreal getPlaybackSpeed();
        void setPlaybackSpeed( qreal playbackSpeed );
        QVariantMap source;
        QVariantMap getSource();
        void setSource( QVariantMap source );
        qreal volume{ 1.0 };
        qreal getVolume();
        void setVolume( qreal volume );
        bool vsync{ true };
        bool getVsync();
        void setVsync( bool vsync );

    signals:
        // Signals for properties
        void controlOutputChanged();
        void globalGamepadChanged();
        void phoenixWindowChanged();
        void videoOutputChanged();

        void playbackSpeedChanged();
        void sourceChanged();
        void volumeChanged();
        void vsyncChanged();
};
