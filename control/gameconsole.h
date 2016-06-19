#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <QThread>
#include <QVariant>
#include <QtGlobal>

#include "pipeline.h"

// Nodes
class AudioOutput;
class SDLManager;
class ControlOutput;
class GlobalGamepad;
class LibretroLoader;
class LibretroRunner;
class LibretroVariableForwarder;
class LibretroVariableModel;
class MicroTimer;
class PhoenixWindow;
class PhoenixWindowNode;
class Remapper;
class RemapperModel;
class SDLUnloader;
class VideoOutput;
class VideoOutputNode;

/*
 * GameConsole's role is twofold:
 * 1. Manage the lifecycle of the global pipeline and every dynamic pipeline (invoked by writing to the source property
 *    and calling load())
 * 2. Pass messages to the pipeline, but only once a dynamic pipeline is established. These messages include commands
 *    like load() (which does some extra management stuff), play(), pause(), etc. along with setters like
 *    playbackSpeed, source, etc.
 *
 * Other than to satisfy those two roles (and to cleanup on shutdown), GameConsole has no other logic nor should it.
 * At its core, its primary role is to simply pass messages to the pipeline from QML.
 *
 * To hook the propogation of these commands down to the running Core, see ControlOutput. Usually this is directly
 * connected to the Core of a particular dynamic pipeline.
 */

class GameConsole : public Node {
        Q_OBJECT

        Q_PROPERTY( int aspectRatioMode READ getAspectRatioMode WRITE setAspectRatioMode NOTIFY aspectRatioModeChanged )
        Q_PROPERTY( qreal playbackSpeed READ getPlaybackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged )
        Q_PROPERTY( QVariantMap source READ getSource WRITE setSource NOTIFY sourceChanged )
        Q_PROPERTY( qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged )
        Q_PROPERTY( bool vsync READ getVsync WRITE setVsync NOTIFY vsyncChanged )
        Q_PROPERTY( QString userDataLocation MEMBER userDataLocation NOTIFY userDataLocationChanged )

    public:
        explicit GameConsole( Node *parent = nullptr );
        ~GameConsole() = default;

    public slots:
        // These are not the same as their Node counterparts
        // Call play() to load a game, call play() again once paused to begin playing it
        // Call stop() to unload then stop a game
        void play();
        void pause();
        void stop();
        void reset();

    private: // Startup
        void load();

        // API-specific loaders
        void loadLibretro();

        // Return true if all global pipeline members from QML are set
        bool globalPipelineReady();

        // Emit Command::GlobalPipelineReady if it's ready to go
        void checkIfGlobalPipelineReady();

        // Return true if a core is loaded and its dynamic pipeline is hooked to the global one
        bool dynamicPipelineReady();

        // QVariantMap that holds property changes that were set before the global pipeline loaded
        QVariantMap pendingPropertyChanges;
        void applyPendingPropertyChanges();

    private: // Cleanup
        // Call the proper unloader based on the API chosen
        // FIXME: Should this ever be used? What if we play() core X, stop() core X, then play() and stop() core Y before
        // core X has called unload()? I'll read source and act as if it's unloading core X
        void unload();

        // Keeps track of session connections so they may be disconnected once emulation ends
        QList<QMetaObject::Connection> sessionConnections;

        // Used to stop the game thread on app quit
        bool quitFlag{ false };

        // API-specific unloaders
        void unloadLibretro();
        void deleteLibretro();

        // Sends deferred delete events to everything, safe (also required) to call after calling your API-specific deleter
        void deleteMembers();

    private: // Members
        // Global pipeline Nodes owned by this class (game thread)
        // When adding a new one, remember to:
        //     - Add to deleteMembers()
        //     - Move to gameThread in the constructor
        MicroTimer *microTimer { nullptr };
        Remapper *remapper { nullptr };
        SDLManager *sdlManager { nullptr };
        SDLUnloader *sdlUnloader { nullptr };

        // Dynamic pipeline Nodes owned by this class (game thread)
        // When adding a new one, remember to:
        //     - Add to deleteMembers() and to delete_____() for each core type (dynamic pipeline) it's used by
        //     - Move to gameThread in the constructor
        AudioOutput *audioOutput { nullptr };
        LibretroLoader *libretroLoader { nullptr };
        LibretroRunner *libretroRunner { nullptr };
        LibretroVariableForwarder *libretroVariableForwarder { nullptr };

    private: // Property getters/setters
        int aspectRatioMode { 0 };
        int getAspectRatioMode();
        void setAspectRatioMode( int aspectRatioMode );
        qreal playbackSpeed { 1.0 };
        qreal getPlaybackSpeed();
        void setPlaybackSpeed( qreal playbackSpeed );
        QVariantMap source;
        QVariantMap getSource();
        void setSource( QVariantMap source );
        qreal volume { 1.0 };
        qreal getVolume();
        void setVolume( qreal volume );
        bool vsync { true };
        bool getVsync();
        void setVsync( bool vsync );
        QString userDataLocation;

    signals: // Property changed notifiers
        void aspectRatioModeChanged();
        void playbackSpeedChanged();
        void sourceChanged();
        void volumeChanged();
        void vsyncChanged();
        void userDataLocationChanged();

};
