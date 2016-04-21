#pragma once

#include <QMutex>
#include <QObject>
#include <QList>

#include <QDebug>

// The INTERFACES macro is a way to provide common signal and slot to any
// QObjects, while getting around Qt's limitation of no multiple inheritance
// of QObjects.

// The 'DerivedClass' argument is the name of the 'subclass', so if you had a
// class named 'Gamepad', you would pass that name to the first argument.

// The Interface class argument will probably always be PipelineNode, but who
// knows.

#define PHX_PIPELINE_INTERFACE( DerivedClass )          \
    Q_SIGNALS:                                          \
        void dataOut( PipelineNode::DataReason reason   \
                     , QMutex *producerMutex            \
                     , void *data                       \
                     , size_t bytes                     \
                     , qint64 timeStamp );              \
                                                        \
    public:                                             \
                                                        \
        PipelineNode &interface() {                     \
            if ( !m_selfConnectionMade ) {              \
                connect( &m_interface                   \
                        , &PipelineNode::dataOut        \
                        , this                          \
                        , &DerivedClass::dataIn );      \
                                                        \
                m_selfConnectionMade = true;            \
            }                                           \
            return m_interface;                         \
        }                                               \
                                                        \
                                                        \
        void lock() const {                             \
            m_interfaceMutex.lock();                    \
        }                                               \
                                                        \
        void unlock() const {                           \
            m_interfaceMutex.unlock();                  \
        }                                               \
                                                        \
        void emitDataOut( PipelineNode::DataReason t_reason         \
                         , void *t_data                             \
                         , size_t t_bytes                           \
                         , qint64 t_timeStamp ) {                   \
                                                                    \
            emit dataOut( t_reason                                  \
                         , &m_interfaceMutex                        \
                         , t_data                                   \
                         , t_bytes                                  \
                         , t_timeStamp);                            \
        }                                                           \
                                                                    \
        void setNodeState( PipelineNode::DataReason t_reason ) {    \
            m_interfaceState = t_reason;                            \
        }                                                           \
                                                                    \
        PipelineNode::DataReason nodeState() const {                \
            return m_interfaceState;                                \
        }                                                           \
                                                                    \
    protected:                                                      \
        PipelineNode m_interface;                                   \
        mutable QMutex m_interfaceMutex;                            \
        PipelineNode::DataReason m_interfaceState{ PipelineNode::State_Changed_To_Stopped_bool }; \
                                                                    \
    private:                                                        \
        bool m_selfConnectionMade{ false };

// PipelineNode is a used as a standalone object inside of any class, in order
// to provide the dataIn() / dataOut() signal and slot functionality.

// All types of objects that are expected to recieve events from the emulation
// pipeline, must use this in order to be synced in.

// Usage is defined by using the INTERFACES( XXX, PipelineNode ), 'XXX' denoting
// the name of the class that you want to be a PipelineNode.

// QObjects only allow single inheritance, this the way that we get around that,
// while keeping common QObject signals and slots.

class PipelineNode : public QObject
{
    Q_OBJECT
public:
    explicit PipelineNode( QObject *parent = 0 )
        : QObject( parent ) {
    }
    ~PipelineNode() = default;

    // The DataReason enum is used as a flag to specifiy the types of
    // dataIn() signals you want to handle. All additional reasons to the enum.
    enum DataReason {
        First,  // Only used for iterating

        Update_Video,
        Update_Audio,
        Update_Input,

        Poll_Input_nullptr,
        Create_Frame_any,

        Format_Changed_ProducerFormat,

        State_Changed_To_Playing_bool,
        State_Changed_To_Paused_bool,
        State_Changed_To_Stopped_bool,
        State_Changed_To_Loading_bool,
        State_Changed_To_Unloading_bool,


        Set_Vsync_bool,
        Set_Playback_Speed_qreal,
        Set_Volume_Level_qreal,
        Set_FrameRate_qreal,
        Set_Source_QVariantMap,
        Set_QML_Loaded_nullptr,


        Last,   // Only used for iterating
    };
    Q_ENUM( DataReason )

signals:
    void dataOut( PipelineNode::DataReason
                  , QMutex *
                  , void *
                  , size_t
                  , qint64 );

};

// This is a wrapper around QObject::connect() so that you can connect the parent
// QObject to the child QObject via the PipelineNode interface,
// aka, classes defined by the INTERFACES macro

template<typename Parent, typename Child>
QMetaObject::Connection connectInterface( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );
    return QObject::connect( t_parent, &Parent::dataOut
                             , &t_child->interface(), &PipelineNode::dataOut );
}

template<typename Parent, typename Child>
QMetaObject::Connection connectInterface( Parent &t_parent, Child &t_child ) {
    return connectInterface( &t_parent, &t_child );
}

// The same as the connectInterface() function, except it disconnects the
// already connected signals.

template<typename Parent, typename Child>
bool disconnectInterface( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );
    return QObject::disconnect( t_parent, &Parent::dataOut
                                , &t_child->interface(), &PipelineNode::dataOut );
}

template<typename Parent, typename Child>
bool disconnectInterface( Parent &t_parent, Child &t_child ) {
    return disconnectFrom( &t_parent, &t_child );
}
