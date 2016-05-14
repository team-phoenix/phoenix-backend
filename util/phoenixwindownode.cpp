#include "phoenixwindownode.h"

#include <QScreen>
#include <QSurface>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>

#include <QThread>
#include "logging.h"

PhoenixWindowNode::PhoenixWindowNode( Node *parent ) : Node( parent ) {
    connect( this, &PhoenixWindowNode::phoenixWindowChanged, this, [ & ]( PhoenixWindow * phoenixWindow ) {
        if( phoenixWindow ) {
            this->phoenixWindow = phoenixWindow;
            phoenixWindow->phoenixWindowNode = this;
            connect( phoenixWindow, &QQuickWindow::frameSwapped, this, &PhoenixWindowNode::frameSwapped );
            connect( phoenixWindow, &QQuickWindow::xChanged, this, &PhoenixWindowNode::geometryChanged );
            connect( phoenixWindow, &QQuickWindow::yChanged, this, &PhoenixWindowNode::geometryChanged );
            connect( phoenixWindow, &QQuickWindow::widthChanged, this, &PhoenixWindowNode::geometryChanged );
            connect( phoenixWindow, &QQuickWindow::heightChanged, this, &PhoenixWindowNode::geometryChanged );
        }
    } );
}

PhoenixWindowNode::~PhoenixWindowNode() {
}

void PhoenixWindowNode::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    switch( command ) {
        case Command::Load: {
            qDebug() << command;
            Q_ASSERT( phoenixWindow );

            // Don't emit load until the dynamic pipeline context is ready
            fireLoad = true;
            checkIfCommandsShouldFire();
            break;
        }

        // Reset all the checkers
        case Command::Unload: {
            emit commandOut( command, data, timeStamp );

            // Reset this so it may be fired again next load
            firedOpenGLContextCommand = false;

            // Reset this so a new load can be called later
            fireLoad = false;
            break;
        }

        case Command::SetGameThread: {
            qDebug() << command;
            emit commandOut( command, data, timeStamp );

            gameThread = data.value<QThread *>();
            checkIfCommandsShouldFire();
            break;
        }

        case Command::HandleDynamicPipelineReady: {
            qDebug() << command;
            Q_ASSERT( phoenixWindow );
            emit commandOut( command, data, timeStamp );
            break;
        }

        case Command::SetVsync: {
            Q_ASSERT( phoenixWindow );
            emit commandOut( command, data, timeStamp );
            phoenixWindow->setVsync( data.toBool() );
            break;
        }

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}

void PhoenixWindowNode::frameSwapped() {
    emit commandOut( Command::Heartbeat, QVariant(), nodeCurrentTime() );
}

void PhoenixWindowNode::geometryChanged() {
    QRect newGeometry = phoenixWindow->geometry();

    if( newGeometry != geometry ) {
        geometry = newGeometry;
        emit commandOut( Command::SetWindowGeometry, geometry, nodeCurrentTime() );
    }
}

void PhoenixWindowNode::checkIfCommandsShouldFire() {
    qDebug() << "Check" << !firedOpenGLContextCommand << ( phoenixWindow && phoenixWindow->dynamicPipelineContext != nullptr ) << fireLoad << ( gameThread != nullptr );

    // Check if it's time to tell the dynamic pipeline about the context and the load
    if( !firedOpenGLContextCommand && phoenixWindow && phoenixWindow->dynamicPipelineContext && fireLoad && gameThread ) {
        firedOpenGLContextCommand = true;

        // Move context to the game thread
        phoenixWindow->dynamicPipelineContext->moveToThread( gameThread );
        phoenixWindow->dynamicPipelineSurface->moveToThread( gameThread );

        // Send everything out
        emit commandOut( Command::SetSurface, QVariant::fromValue<QOffscreenSurface *>( phoenixWindow->dynamicPipelineSurface ), nodeCurrentTime() );
        emit commandOut( Command::SetOpenGLContext, QVariant::fromValue<QOpenGLContext *>( phoenixWindow->dynamicPipelineContext ), nodeCurrentTime() );
        emit commandOut( Command::SetOpenGLFBO, QVariant::fromValue<void *>( static_cast<void *>( phoenixWindow->dynamicPipelineFBO ) ), nodeCurrentTime() );
        emit commandOut( Command::Load, QVariant(), nodeCurrentTime() );
    }
}
