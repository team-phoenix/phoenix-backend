#include "mousemanager.h"
#include "keyboardmouselistener.h"

#include "logging.h"

MouseManager::MouseManager( QObject *parent )
    : Node( parent ) {

}

void MouseManager::setListener( KeyboardMouseListener *listener ) {
    this->listener = listener;
    connect( listener, &KeyboardMouseListener::mousePressed, this, &MouseManager::mousePressd );
    connect( listener, &KeyboardMouseListener::mouseReleased, this, &MouseManager::mouseReleased );
    connect( listener, &KeyboardMouseListener::mouseMoved, this, &MouseManager::mouseMoved );
}

void MouseManager::mousePressd( QPointF point , Qt::MouseButtons buttons ) {
    mouse.position = point;
    mouse.buttons = buttons;
}

void MouseManager::mouseReleased( QPointF point, Qt::MouseButtons buttons ) {
    mouse.position = point;
    mouse.buttons = buttons;
}

void MouseManager::mouseMoved( QPointF point, Qt::MouseButtons buttons ) {
    mouse.position = point;
    mouse.buttons = buttons;
}

void MouseManager::commandIn( Command command, QVariant data, qint64 timeStamp ) {

    switch( command ) {
        case Command::Heartbeat: {
            // Send mouse state on its way
            {
                // Copy mouse into buffer
                mutex.lock();
                mouseBuffer[ mouseBufferIndex ] = mouse;
                mutex.unlock();

                // Send buffer on its way
                emit dataOut( DataType::MouseInput, &mutex, &mouseBuffer[ mouseBufferIndex ], 0, QDateTime::currentMSecsSinceEpoch() );

                // Increment the index
                mouseBufferIndex = ( mouseBufferIndex + 1 ) % 100;
            }

            // Inject heartbeat into child nodes' event queues *after* input data
            emit commandOut( command, data, timeStamp );
            break;
        }

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }

    }

}
