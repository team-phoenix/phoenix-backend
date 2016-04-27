#include "node.h"

Node::Node( QObject *parent ) : QObject( parent ) {

}

void Node::controlIn( Command command, QVariant data, qint64 timeStamp ) {
    emit controlOut( command, data, timeStamp );
}

void Node::dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    emit dataOut( type, mutex, data, bytes, timeStamp );
}
