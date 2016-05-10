#include "touchmanager.h"
#include "keyboardlistener.h"

#include <QDebug>

TouchManager::TouchManager(QObject *parent)
    : Node( parent) {

}

void TouchManager::setListener( KeyboardListener *t_listener ) {
    m_listener = t_listener;
    connect( m_listener, &KeyboardListener::mousePressed, this, &TouchManager::handleMousePress );
    connect( m_listener, &KeyboardListener::mouseReleased, this, &TouchManager::handleMouseRelease );
}

void TouchManager::handleMousePress(QPointF t_point) {
    m_touchState.point = t_point;
    m_touchState.pressed = true;
}

void TouchManager::handleMouseRelease(QPointF t_point) {
    m_touchState.point = t_point;
    m_touchState.pressed = false;
}

void TouchManager::commandIn( Command command, QVariant data, qint64 timeStamp) {

    switch( command ) {
        case Command::Heartbeat: {
            // Send keyboard state on its way
            {
                // Copy keyboard into buffer
                //m_mutex.lock();
                //keyboardBuffer[ keyboardBufferIndex ] = keyboard;
                //m_mutex.unlock();

                // Send buffer on its way

                emit dataOut( DataType::TouchInput, &m_mutex, &m_touchState, 0, QDateTime::currentMSecsSinceEpoch() );

            }

            // Inject heartbeat into child nodes' event queues *after* input data
            Node::commandIn( command, data, timeStamp );

            break;
        }

        default: {
            Node::commandIn( command, data, timeStamp );
            break;
        }

    }

}
