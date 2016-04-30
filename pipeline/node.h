#pragma once

#include <QMutex>
#include <QObject>
#include <QVariant>
#include <QDateTime>

// FIXME: start using once producer.h/control.h is no longer needed
// #include "pipelinecommon.h"

/*
 * A node in the pipeline tree. This class defines a set of signals and slots common to each node.
 */

class Node : public QObject {
        Q_OBJECT

    public:
        explicit Node( QObject *parent = 0 );

        enum class Command {
            // State setters
            // FIXME: Redundant?
            Stop,
            Load,
            Play,
            Pause,
            Unload,
            Reset,

            // Run pipeline for a frame
            Heartbeat,

            // Inform consumers about heartbeat rate
            HeartbeatRate,

            // Set FPS of the monitor. Used for when vsync is on (track with SetVsync)
            HostFPS,

            // Set FPS of the core in this pipeline. May not exist!
            CoreFPS,

            // Change consumer format
            AudioFormat,
            VideoFormat,
            InputFormat,

            // Is this Core instance pausable? NOTE: "pausable" means whether or not you can *enter* State::PAUSED, not leave.
            // Core will ALWAYS enter State::PAUSED after State::LOADING regardless of this setting
            // bool
            SetPausable,

            // Multiplier of the system's native framerate, if any. If rewindable, it can be any real number. Otherwise, it must
            // be positive and nonzero
            // qreal
            SetPlaybackSpeed,

            // Core subclass-defined info specific to this session (ex. Libretro: core, game, system and save paths)
            // QVariantMap
            SetSource,

            // Is this Core instance resettable? If true, this usually means you can "soft reset" instead of fully resetting
            // the state machine by cycling through the deinit then init states
            // Read-only
            SetResettable,

            // Is this Core instance rewindable? If true, playbackSpeed may go to and below 0 to make the game move backwards
            // bool
            SetRewindable,

            // Set volume. Range: [0.0, 1.0]
            // qreal
            SetVolume,

            // Set vsync mode
            SetVsync,

            // Handle a new controller being added/removed (instanceID provided)
            // int
            ControllerAdded,
            ControllerRemoved
        };
        Q_ENUM( Command )

        enum class DataType {
            Video,
            Audio,
            Input,
            TouchInput,
        };
        Q_ENUM( DataType )

        // FIXME: Redundant?
        enum class State {
            Stopped,
            Loading,
            Playing,
            Paused,
            Unloading,
        };
        Q_ENUM( State )

    signals:
        void commandOut( Command command, QVariant data, qint64 timeStamp );
        void dataOut( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp );

    public slots:
        // By default, these slots will relay the signals given to them to their children

        // FIXME: Use these or make an explicit "repeat command" function for classes that override these?
        virtual void commandIn( Command command, QVariant data, qint64 timeStamp );
        virtual void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp );
};

// Convenience functions for easily connecting and disconnecting nodes

QList<QMetaObject::Connection> connectNodes( Node *t_parent, Node *t_child );

QList<QMetaObject::Connection> connectNodes( Node &t_parent, Node &t_child );

bool disconnectNodes( Node *t_parent, Node *t_child );

bool disconnectNodes( Node &t_parent, Node &t_child );