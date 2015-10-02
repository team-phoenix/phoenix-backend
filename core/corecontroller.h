#ifndef CORECONTROLLER_H
#define CORECONTROLLER_H

#include <QtCore>
#include <QtQuick>

#include "coremodel.h"
#include "logging.h"

/*
 * CoreController is a QML type that manages the execution of a game.
 *
 * Internally, CoreController is just a QML wrapper for the underlying CoreModel. It will provide a thread for CoreModel
 * to operate under, and exposes a set of properties and methods for use by QML code. It is also the layer in which error
 * checking and recovery is preformed.
 *
 * From the perspective of QML, the interface exposed is similar to that of a game console. See docs for state property
 * for more info on how to run and manage the game session.
 */

class CoreController : public QObject {
        Q_OBJECT

        // Properties

        // error : enumeration
        /*
         * List of possible errors:
         * loading:
         * invalid value for type (not a string, a string but not a possible value)
         * invalid value for core/game (not a string, if it's a string it'll be evaluated later)
         * out of memory
         * unable to open core or game (I/O error, permissions, file not there
         * function called during the wrong state for that function
         *
         */

        // errorString : string

        // Master mute toggle, true iff volume is 0.0
        Q_PROPERTY( bool muted MEMBER muted NOTIFY signalMutedChanged )

        // playbackRate : real
        // Default: 1.0
        // Nonzero when playing, 0.0 when paused
        // Positive is forward playback, negative is backward playback (limited to size and granularity of rewind buffer)
        // Requested speed is not guarantied to be possible, and once rewind buffer empties game will be paused
        Q_PROPERTY( qreal playbackRate MEMBER playbackRate NOTIFY signalPlaybackRateChanged )

        // resettable : bool

        // rewindable : bool

        // Information about the game being played
        // The dictionary must contain the following:
        //     "type" : string -- The subsystem to be used in this session (mandatory)
        //
        //     Mandatory if "type" is set to "libretro":
        //     "core" : string -- Absolute path to a libretro core
        //     "game" : string -- Absolute path to a libretro-compatable game file
        Q_PROPERTY( QVariantMap source MEMBER source NOTIFY signalSourceChanged )

        // Current state. Possible values: "init", "stopped", "loading", "paused", "playing", "unloading", and "error"
        //     "init": Still in the constructor, probably allocating memory or something
        //     "stopped": Nothing's loaded. Give CoreControl a game to play then call load()
        //     "loading": Game is currently loading.
        //     "paused": Game fully loaded, not running. Call play() to resume game or stop() to stop playing
        //     "playing": Game fully loaded, running. Call pause() to pause game or stop() to stop playing
        //     "unloading": Game is shutting down. It could be doing stuff like saving game data back to disk or freeing memory
        //     "error": Something went wrong. Check the error property and handle accordingly. stop() the session once
        //              you've handled the situation. CoreController will clean up memory and should be ready to launch
        //              again.
        Q_PROPERTY( QString state READ getState NOTIFY signalStateChanged )


        // Master volume, range: [0.0, 1.0]
        Q_PROPERTY( qreal volume MEMBER volume NOTIFY signalVolumeChanged )

    public:
        explicit CoreController( QObject *parent = 0 );

        // Attempt to load the given game (see source property)
        void load();

        // Standard playback controls
        void play();
        void pause();
        void stop();

        // Reset underlying game session, if possible
        void reset();

    signals:
        // Property change notifiers
        void signalMutedChanged( bool muted );
        void signalPlaybackRateChanged( bool playbackRate );
        void signalSourceChanged( QVariantMap source );
        void signalStateChanged( QString state );
        void signalVolumeChanged( qreal volume );

    public slots:

    private:
        // Properties
        bool muted;
        qreal playbackRate;
        QVariantMap source;
        QString state;
        qreal volume;

        // Property getters
        QString getState();

        // CoreModel and its thread
        QThread *coreThread;
        CoreModel *coreModel;

};

#endif // CORECONTROLLER_H
