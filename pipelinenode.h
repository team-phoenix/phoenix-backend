#pragma once

#include <QMutex>
#include <QObject>
#include <QList>
#include <QVariant>

#include <QDebug>

// The INTERFACES macro is a way to provide common signal and slot to any
// QObjects, while getting around Qt's limitation of no multiple inheritance
// of QObjects.

// The 'DerivedClass' argument is the name of the 'subclass', so if you had a
// class named 'Gamepad', you would pass that name to the first argument.

// The Interface class argument will probably always be PipelineNode, but who
// knows.

#define PHX_PIPELINE_INTERFACE( DerivedClass )          \
                                                        \
    Q_SIGNALS:                                          \
        void dataOut( DataReason reason                 \
                     , QMutex *producerMutex            \
                     , void *data                       \
                     , size_t bytes                     \
                     , qint64 timeStamp );              \
                                                        \
        void controlOut( Command t_cmd                  \
                        , QVariant data );              \
                                                        \
        void stateOut( PipeState t_state );             \
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
                connect( &m_interface                   \
                        , &PipelineNode::controlOut     \
                        , this                          \
                        , &DerivedClass::controlIn );   \
                                                        \
                connect( &m_interface                   \
                        , &PipelineNode::stateOut       \
                        , this                          \
                        , &DerivedClass::stateIn );     \
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
        QMutex &interfaceMutex() {                \
            return m_interfaceMutex;                    \
        }                                               \
                                                        \
        void emitDataOut( DataReason t_reason           \
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
        void setPipeState( PipeState t_state ) {                    \
            m_state = t_state;                                      \
            emit stateOut( t_state );                               \
        }                                                           \
                                                                    \
        PipeState pipeState() const {                                       \
            return m_state;                                         \
        }                                                           \
                                                                    \
    protected:                                                      \
        PipelineNode m_interface;                                   \
        mutable QMutex m_interfaceMutex;                            \
        PipeState m_state{ PipeState::Stopped }; \
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

enum class Command {
    First = 0,

    Format_Changed_ProducerFormat,
    Set_Vsync_bool,
    Set_Playback_Speed_qreal,
    Set_Volume_Level_qreal,
    Set_FrameRate_qreal,
    Set_Source_QVariantMap,
    Set_QML_Loaded_nullptr,

    UpdateAVFormat,

    Last,
};

enum class PipeState {
    Playing,
    Paused,
    Stopped,
    Loading,
    Unloading,
};

enum class DataReason {

    UpdateVideo,
    UpdateAudio,
    UpdateInput,

    PollInput,

};

class PipelineNode : public QObject
{
    Q_OBJECT
public:
    explicit PipelineNode( QObject *parent = nullptr )
        : QObject( parent ) {
    }
    ~PipelineNode() = default;


    // The DataReason enum is used as a flag to specifiy the types of
    // dataIn() signals you want to handle. All additional reasons to the enum.


signals:
    void dataOut( DataReason
                  , QMutex *
                  , void *
                  , size_t
                  , qint64 );

    void controlOut( Command
                     , QVariant );

    void stateOut( PipeState );

};

// This is a wrapper around QObject::connect() so that you can connect the parent
// QObject to the child QObject via the PipelineNode interface,
// aka, classes defined by the INTERFACES macro

template<typename Parent, typename Child>
QList<QMetaObject::Connection> connectInterface( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );

    return {
        QObject::connect( t_parent, &Parent::dataOut
                                     , &t_child->interface(), &PipelineNode::dataOut )
        , QObject::connect( t_parent, &Parent::controlOut
                            , &t_child->interface(), &PipelineNode::controlOut )

        , QObject::connect( t_parent, &Parent::stateOut
                            , &t_child->interface(), &PipelineNode::stateOut )
    };
}

template<typename Parent, typename Child>
QList<QMetaObject::Connection> connectInterface( Parent &t_parent, Child &t_child ) {
    return connectInterface( &t_parent, &t_child );
}

// The same as the connectInterface() function, except it disconnects the
// already connected signals.

template<typename Parent, typename Child>
bool disconnectInterface( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );
    return ( QObject::disconnect( t_parent, &Parent::dataOut
                                , &t_child->interface(), &PipelineNode::dataOut )
             && QObject::disconnect( t_parent, &Parent::controlOut
                                 , &t_child->interface(), &PipelineNode::controlOut )
             && QObject::disconnect( t_parent, &Parent::stateOut
                                  , &t_child->interface(), &PipelineNode::stateOut ) );
}

template<typename Parent, typename Child>
bool disconnectInterface( Parent &t_parent, Child &t_child ) {
    return disconnectFrom( &t_parent, &t_child );
}

Q_DECLARE_METATYPE( PipeState )
Q_DECLARE_METATYPE( Command )
Q_DECLARE_METATYPE( DataReason )
