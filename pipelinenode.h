#pragma once

#include <QObject>
#include <QMutex>
#include <QObject>
#include <QList>
#include <QVariant>

#include <QDebug>

// Some enums for the types of data/commands that flow across the pipeline
// This Q_GADGET contains the enums so we can utilize the Q_ENUM() macro to pretty-print them
class PipelineEnums {
    Q_GADGET

    public:
        enum class Command {
            // State setters
            Play,
            Pause, // Do not call if not pausable
            Stop,
            Load,
            Unload,

            // Run emulation for a frame
            Heartbeat,

            Format_Changed_ProducerFormat,
            Set_Vsync_bool,
            Set_Playback_Speed_qreal,
            Set_Volume_Level_qreal,
            Set_FrameRate_qreal,
            Set_Source_QVariantMap,
            Set_QML_Loaded_nullptr,

            UpdateAVFormat,
        };

        enum class PipelineState {
            Playing,
            Paused,
            Stopped,
            Loading,
            Unloading,
        };

        enum class DataType {
            Video,
            Audio,
            Input,

            PollInput,
        };
        Q_ENUM( Command )
        Q_ENUM( PipelineState )
        Q_ENUM( DataType )
};

// Make the enums available with only one level of scoping necessary
using Command = PipelineEnums::Command;
using PipelineState = PipelineEnums::PipelineState;
using DataType = PipelineEnums::DataType;

// The PHX_PIPELINE_NODE_* macros and the PipelineNode class are a way to provide common signals and slots to all pipeline
// nodes while getting around Qt's limitation of multiple inheritance of QObjects.
// To make a valid node subclass:
// 1. Make your class a subclass of "public PipelineNode" (shown below)
// 2. Use the signal macro and at least one of each slot macro

// Put the following macros in your subclass:

// (required) Place after "signals:" in your node class
#define PHX_PIPELINE_NODE_SIGNALS                                                                                      \
    void dataOut( DataType reason, QMutex *producerMutex, void *data, size_t bytes, qint64 timeStamp );                \
    void controlOut( Command command, QVariant data );                                                                 \
    void stateOut( PipelineState state );                                                                              \

// (required) Choose one of the following and place after "public slots:" in your node class
#define PHX_PIPELINE_NODE_SLOT_DATAIN_PURE_VIRTUAL                                                                     \
    virtual void dataIn( DataType reason, QMutex *producerMutex, void *data, size_t bytes, qint64 timeStamp ) = 0;     \

#define PHX_PIPELINE_NODE_SLOT_DATAIN_DEFAULT                                                                          \
    void dataIn( DataType reason, QMutex *producerMutex, void *data, size_t bytes, qint64 timeStamp ) override {       \
        emit dataOut( reason, producerMutex, data, bytes, timeStamp );                                                 \
    }                                                                                                                  \
                                                                                                                       \

#define PHX_PIPELINE_NODE_SLOT_DATAIN_OVERRIDE                                                                         \
    void dataIn( DataType reason, QMutex *producerMutex, void *data, size_t bytes, qint64 timeStamp ) override;        \

// (required) Choose one of the following and place after "public slots:" in your node class
#define PHX_PIPELINE_NODE_SLOT_CONTROLIN_PURE_VIRTUAL                                                                  \
    virtual void controlIn( Command command, QVariant data ) = 0;                                                      \

#define PHX_PIPELINE_NODE_SLOT_CONTROLIN_DEFAULT                                                                       \
    void controlIn( Command command, QVariant data ) override {                                                        \
        emit controlOut( command, data );                                                                              \
    }                                                                                                                  \
                                                                                                                       \

#define PHX_PIPELINE_NODE_SLOT_CONTROLIN_OVERRIDE                                                                      \
    void controlIn( Command command, QVariant data ) override;                                                         \

// (required) Choose one of the following and place after "public slots:" in your node class
#define PHX_PIPELINE_NODE_SLOT_STATEIN_PURE_VIRTUAL                                                                    \
    virtual void stateIn( PipelineState state ) = 0;                                                                   \

#define PHX_PIPELINE_NODE_SLOT_STATEIN_DEFAULT                                                                         \
    void stateIn( PipelineState state ) override {                                                                     \
        m_state = state;                                                                                               \
        emit stateOut( state );                                                                                        \
    }                                                                                                                  \
                                                                                                                       \

#define PHX_PIPELINE_NODE_SLOT_STATEIN_OVERRIDE                                                                        \
    void stateIn( PipelineState state ) override;                                                                      \

// Extend your node with this subclass
class PipelineNode {
    public:
        PipelineNode();

 /* signals:
        // Signals used by all nodes, use PHX_PIPELINE_NODE_SIGNALS to declare them
        void dataOut( DataType reason, QMutex *producerMutex, void *data, size_t bytes, qint64 timeStamp );
        void controlOut( Command command, QVariant data );
        void stateOut( PipelineState state ); */

    public /*slots*/:
        // Slots available on all nodes, use PHX_PIPELINE_NODE_SLOT_* to declare them
        virtual void dataIn( DataType reason, QMutex *producerMutex, void *data, size_t bytes, qint64 timeStamp ) = 0;
        virtual void controlIn( Command command, QVariant data ) = 0;
        virtual void stateIn( PipelineState state ) = 0;

    protected:
        mutable QMutex m_interfaceMutex;
        PipelineState m_state{ PipelineState::Stopped };

        // Convenience methods for data producers

        // Use to lock and unlock the included mutex
        void lock() const {
            m_interfaceMutex.lock();
        }

        void unlock() const {
            m_interfaceMutex.unlock();
        }

        QMutex &interfaceMutex() {
            return m_interfaceMutex;
        }

        PipelineState pipeState() const {
            return m_state;
        }
};

// Convenience functions for producers
// Must be defined as macros as PipelineNode is not a QObject

#define emitDataOut( t_reason, t_data, t_bytes, t_timeStamp )                                                          \
    emit dataOut( t_reason, &m_interfaceMutex, t_data, t_bytes, t_timeStamp );                                         \

#define setPipeState( t_state )                                                                                        \
    m_state = t_state;                                                                                                 \
    emit stateOut( t_state );                                                                                          \

// This is a wrapper around QObject::connect() so that you can connect the parent QObject to the child QObject via the
// PipelineNode interface, aka, classes defined by the PHX_PIPELINE_NODE macro

template<typename Parent, typename Child>
QList<QMetaObject::Connection> connectInterface( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );

    return {
        QObject::connect( t_parent, &Parent::dataOut, t_child, &Child::dataIn ),
        QObject::connect( t_parent, &Parent::controlOut, t_child, &Child::controlIn ),
        QObject::connect( t_parent, &Parent::stateOut, t_child, &Child::stateIn )
    };
}

template<typename Parent, typename Child>
QList<QMetaObject::Connection> connectInterface( Parent &t_parent, Child &t_child ) {
    return connectInterface( &t_parent, &t_child );
}

// The same as the connectInterface() function, except it disconnects the already connected signals.

template<typename Parent, typename Child>
bool disconnectInterface( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );
    return ( QObject::disconnect( t_parent, &Parent::dataOut, t_child, &Child::dataIn ) &&
             QObject::disconnect( t_parent, &Parent::controlOut, t_child, &Child::controlIn ) &&
             QObject::disconnect( t_parent, &Parent::stateOut, t_child, &Child::stateIn )
           );
}

template<typename Parent, typename Child>
bool disconnectInterface( Parent &t_parent, Child &t_child ) {
    return disconnectFrom( &t_parent, &t_child );
}
