#include "keyboardmanager.h"

#include "keyboardlistener.h"

#include <QDebug>
#include <QtGlobal>

void KeyboardManager::connectKeyboardInput( KeyboardListener *keyboardInput ) {
    Q_ASSERT( keyboardInput );
    connect( keyboardInput, &KeyboardListener::keyPressed, this, &KeyboardManager::keyPressed );
    connect( keyboardInput, &KeyboardListener::keyReleased, this, &KeyboardManager::keyReleased );
}

void KeyboardManager::keyPressed( int key ) {
    insertState( key, true );
}

void KeyboardManager::keyReleased( int key ) {
    insertState( key, false );
}

void KeyboardManager::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    switch( command ) {
        case Command::Heartbeat: {
            // Send keyboard state on its way
            {
                // Copy keyboard into buffer
                mutex.lock();
                keyboardBuffer[ keyboardBufferIndex ] = keyboard;
                mutex.unlock();

                // Send buffer on its way
                emit dataOut( DataType::KeyboardInput, &mutex, &keyboardBuffer[ keyboardBufferIndex ], 0, QDateTime::currentMSecsSinceEpoch() );

                // Increment the index
                keyboardBufferIndex = ( keyboardBufferIndex + 1 ) % 100;

                // Reset the keyboard
                keyboard.head = 0;
                keyboard.tail = 0;
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

void KeyboardManager::insertState( int key, bool state ) {
    // Insert key and state into next location
    keyboard.key[ keyboard.tail ] = key;
    keyboard.pressed[ keyboard.tail ] = state;

    // Recalculate head and tail
    keyboard.tail = ( keyboard.tail + 1 ) % 128;

    // Advance head forward if tail will overwrite it next time
    if( keyboard.tail == keyboard.head ) {
        keyboard.head = ( keyboard.head + 1 ) % 128;
    }
}
