#pragma once

#include <QMutex>
#include <QObject>
#include <QVariant>
#include <QDateTime>

// FIXME: start using once producer.h is no longer needed
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
            Stop,
            Load,
            Play,
            Pause,
            Unload,
            Reset,

            // Called just before app quits
            Quit,

            // Run pipeline for a frame
            Heartbeat,

            // Inform consumers about heartbeat rate
            HeartbeatRate,

            // Change consumer format
            AudioFormat,
            VideoFormat,
            InputFormat,
        };
        Q_ENUM( Command )

        enum class DataType {
            Video,
            Audio,
            Input,
        };
        Q_ENUM( DataType )

    signals:
        void controlOut( Command command, QVariant data, qint64 timeStamp );
        void dataOut( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp );

    public slots:
        // By default, these slots will relay the signals given to them to their children
        virtual void controlIn( Command command, QVariant data, qint64 timeStamp );
        virtual void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp );
};

// Convenience functions for easily connecting and disconnecting nodes

QList<QMetaObject::Connection> connectNodes( Node *t_parent, Node *t_child );

QList<QMetaObject::Connection> connectNodes( Node &t_parent, Node &t_child );

bool disconnectNodes( Node *t_parent, Node *t_child );

bool disconnectNodes( Node &t_parent, Node &t_child );
