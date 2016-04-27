#pragma once

#include <QObject>
#include <QVariant>

// FIXME: start using once producer.h is no longer needed
// #include "pipelinecommon.h"

class QMutex;

/*
 * A node in the pipeline tree. Defines a set of signals and slots common to each node.
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
        virtual void controlIn( Command command, QVariant data, qint64 timeStamp );
        virtual void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp );
};
