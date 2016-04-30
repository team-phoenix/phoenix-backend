#include "phoenixwindownode.h"

#include <QDateTime>
#include <QScreen>

PhoenixWindowNode::PhoenixWindowNode( Node *parent ) : Node( parent ) {
    connect( this, &PhoenixWindowNode::phoenixWindowChanged, this, [ & ]( PhoenixWindow * phoenixWindow ) {
        if( phoenixWindow ) {
            this->phoenixWindow = phoenixWindow;
            connect( phoenixWindow, &QQuickWindow::frameSwapped, this, &PhoenixWindowNode::frameSwapped );
        }
    } );
}

void PhoenixWindowNode::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    switch( command ) {
        case Command::SetVsync: {
            Q_ASSERT( phoenixWindow );
            phoenixWindow->setVsync( data.toBool() );
        }

        default: {
            break;
        }
    }
}

void PhoenixWindowNode::frameSwapped() {
    emit commandOut( Command::Heartbeat, QVariant(), QDateTime::currentMSecsSinceEpoch() );
}
