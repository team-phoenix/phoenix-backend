#include "phoenixwindownode.h"

#include <QDateTime>
#include <QScreen>

PhoenixWindowNode::PhoenixWindowNode( Node *parent ) : Node( parent ) {
    connect( this, &PhoenixWindowNode::phoenixWindowChanged, this, [ & ]( PhoenixWindow * phoenixWindow ) {
        if( phoenixWindow ) {
            this->phoenixWindow = phoenixWindow;
            connect( phoenixWindow, &QQuickWindow::frameSwapped, this, &PhoenixWindowNode::frameSwapped );
            connect( phoenixWindow, &QQuickWindow::xChanged, this, &PhoenixWindowNode::geometryChanged );
            connect( phoenixWindow, &QQuickWindow::yChanged, this, &PhoenixWindowNode::geometryChanged );
            connect( phoenixWindow, &QQuickWindow::widthChanged, this, &PhoenixWindowNode::geometryChanged );
            connect( phoenixWindow, &QQuickWindow::heightChanged, this, &PhoenixWindowNode::geometryChanged );
        }
    } );
}

void PhoenixWindowNode::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    switch( command ) {
        case Command::SetVsync: {
            Q_ASSERT( phoenixWindow );
            phoenixWindow->setVsync( data.toBool() );
            break;
        }

    case Command::DynamicPipelineReady: {
        Q_ASSERT( phoenixWindow );

        break;
    }

        default: {
            break;
        }
    }
}

void PhoenixWindowNode::frameSwapped() {
    emit commandOut( Command::Heartbeat, QVariant(), QDateTime::currentMSecsSinceEpoch() );
}

void PhoenixWindowNode::geometryChanged()
{
    QRect newGeometry = phoenixWindow->geometry();
    if( newGeometry != geometry ) {
        geometry = newGeometry;
        emit commandOut( Command::SetWindowGeometry, geometry, QDateTime::currentMSecsSinceEpoch() );
    }
}
