#include "node.h"

Node::Node( QObject *parent ) : QObject( parent ) {

}

void Node::controlIn( Command command, QVariant data, qint64 timeStamp ) {
    emit controlOut( command, data, timeStamp );
}

void Node::dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    emit dataOut( type, mutex, data, bytes, timeStamp );
}

template<typename Parent, typename Child>
QList<QMetaObject::Connection> connectNodes( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );

    return {
        QObject::connect( t_parent, &Parent::dataOut, t_child, &Child::dataIn ),
        QObject::connect( t_parent, &Parent::controlOut, t_child, &Child::controlIn )
    };
}

template<typename Parent, typename Child>
QList<QMetaObject::Connection> connectNodes( Parent &t_parent, Child &t_child ) {
    return connectNodes( &t_parent, &t_child );
}

template<typename Parent, typename Child>
bool disconnectNodes( Parent *t_parent, Child *t_child ) {
    Q_ASSERT( t_parent != nullptr );
    Q_ASSERT( t_child != nullptr );
    return ( QObject::disconnect( t_parent, &Parent::dataOut, t_child, &Child::dataIn ) &&
             QObject::disconnect( t_parent, &Parent::controlOut, t_child, &Child::controlIn )
           );
}

template<typename Parent, typename Child>
bool disconnectNodes( Parent &t_parent, Child &t_child ) {
    return disconnectNodes( &t_parent, &t_child );
}
