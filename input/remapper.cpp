#include "remapper.h"

Remapper::Remapper( Node *parent ) : Node( parent ) {

}

// Public slots

void Remapper::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    switch( command ) {
        case Command::ControllerAdded: {
            Gamepad gamepad = data.value<Gamepad>();
            QString GUID( QByteArray( ( const char * )gamepad.GUID.data, 16 ).toHex() );

            // Add to map if it hasn't been encountered yet
            // Otherwise, just increment the count
            if( !GUIDCount.contains( GUID ) ) {
                GUIDCount[ GUID ] = 1;
                emit controllerAdded( GUID );
            } else {
                GUIDCount[ GUID ]++;
            }

            // FIXME: Replace with something that reads all known mappings from disk, in the constructor
            // For now, just init the remap with default mappings
            for( int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++ ) {
                remapData[ GUID ][ i ] = i;
            }

            break;
        }

        case Command::ControllerRemoved: {
            Gamepad gamepad = data.value<Gamepad>();
            QString GUID( QByteArray( ( const char * )gamepad.GUID.data, 16 ).toHex() );
            GUIDCount[ GUID ]--;

            // Remove from map if no longer there
            if( GUIDCount[ GUID ] == 0 ) {
                GUIDCount.remove( GUID );
                emit controllerRemoved( GUID );
            }

            // If in remap mode, make sure the GUID in question still exists, exit remap mode if not
            if( remapMode && !GUIDCount.contains( GUID ) ) {
                qCWarning( phxInput ) << "No controllers with GUID" << GUID << "remaining, exiting remap mode!";
                remapMode = false;
                emit remapModeEnd( GUID, "ERROR", "ERROR" );
            }

            break;
        }

        // FIXME: Remove
        case Command::Play: {
            // Enter remapping mode for testing
            qDebug() << "Entering remap mode: enter a remapping for A";
            remapModeBegin( "78696e70757401000000000000000000", "A" );
        }

        default: {
            break;
        }
    }
}

void Remapper::dataIn( Node::DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    switch( type ) {
        case DataType::Input: {
            // Copy incoming data to our own buffer
            Gamepad gamepad;
            {
                mutex->lock();
                gamepad = *( Gamepad * )data;
                mutex->unlock();
            }

            // If we are in remap mode, check for a button press from the stored GUID, and remap the stored button to that button
            QString GUID( QByteArray( ( const char * )gamepad.GUID.data, 16 ).toHex() );
            {
                if( remapMode && GUID == remapModeGUID ) {
                    // Find a button press, the first one we encounter will be the new remapping
                    for( int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++ ) {
                        if( gamepad.button[ i ] ) {
                            // TODO: Store this mapping to disk
                            qCDebug( phxInput ) << "Button" << buttonToString( remapModeButton )
                                                << "remapped to" << buttonToString( i ) << "for GUID" << GUID;

                            // Store the new remapping internally
                            remapData[ GUID ][ remapModeButton ] = i;

                            // Tell the model we're done
                            remapMode = false;
                            ignoreMode = true;
                            ignoreModeButton = i;
                            qCDebug( phxInput ) << remapData[ GUID ];
                            emit remapModeEnd( GUID, buttonToString( remapModeButton ), buttonToString( i ) );
                            break;
                        }
                    }

                    // Do not go any further in this data handler
                    return;
                } else if( remapMode ) {
                    // Do not go any further in this data handler
                    return;
                }
            }

            // If ignoreButton is set, the user hasn't let go of the button they were remapping to
            // Do not let the button go through until they let go
            if( ignoreMode ) {
                if( gamepad.button[ ignoreModeButton ] == SDL_PRESSED ) {
                    gamepad.button[ ignoreModeButton ] = SDL_RELEASED;
                } else {
                    ignoreMode = false;
                }
            }

            // Remap button states according to stored data
            Gamepad remappedGamepad = gamepad;
            {
                // Clear remappedGamepad's states
                for( int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++ ) {
                    remappedGamepad.button[ i ] = SDL_RELEASED;
                }

                QMap<int, int> remap = remapData[ GUID ];

                for( int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++ ) {
                    // Read remapped button id from stored remap data
                    int remappedButtonID = remap[ i ];

                    // Write this button's state to the remapped button ID
                    remappedGamepad.button[ remappedButtonID ] |= gamepad.button[ i ];
                }
            }

            // Send updated data out
            {
                // Copy current gamepad into buffer
                this->mutex.lock();
                gamepadBuffer[ gamepadBufferIndex ] = remappedGamepad;
                this->mutex.unlock();

                // Send buffer on its way
                emit dataOut( DataType::Input, &( this->mutex ), ( void * )( &gamepadBuffer[ gamepadBufferIndex ] ), 0, QDateTime::currentMSecsSinceEpoch() );

                // Increment the index
                gamepadBufferIndex = ( gamepadBufferIndex + 1 ) % 100;
            }
            break;
        }

        default: {
            emit dataOut( type, mutex, data, bytes, timeStamp );
            break;
        }
    }
}

void Remapper::remapModeBegin( QString GUID, QString button ) {
    remapMode = true;
    remapModeGUID = GUID;
    remapModeButton = stringToButton( button );
}

// Private

QString Remapper::buttonToString( int button ) {
    switch( button ) {
        case SDL_CONTROLLER_BUTTON_A: {
            return QStringLiteral( "A" );
        }

        case SDL_CONTROLLER_BUTTON_B: {
            return QStringLiteral( "B" );
        }

        case SDL_CONTROLLER_BUTTON_X: {
            return QStringLiteral( "X" );
        }

        case SDL_CONTROLLER_BUTTON_Y: {
            return QStringLiteral( "Y" );
        }

        case SDL_CONTROLLER_BUTTON_BACK: {
            return QStringLiteral( "Back" );
        }

        case SDL_CONTROLLER_BUTTON_GUIDE: {
            return QStringLiteral( "Guide" );
        }

        case SDL_CONTROLLER_BUTTON_START: {
            return QStringLiteral( "Start" );
        }

        case SDL_CONTROLLER_BUTTON_LEFTSTICK: {
            return QStringLiteral( "L3" );
        }

        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: {
            return QStringLiteral( "R3" );
        }

        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: {
            return QStringLiteral( "L" );
        }

        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: {
            return QStringLiteral( "R" );
        }

        case SDL_CONTROLLER_BUTTON_DPAD_UP: {
            return QStringLiteral( "Up" );
        }

        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
            return QStringLiteral( "Down" );
        }

        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: {
            return QStringLiteral( "Left" );
        }

        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: {
            return QStringLiteral( "Right" );
        }

        default: {
            return QStringLiteral( "ERROR" );
        }
    }
}

int Remapper::stringToButton( QString button ) {
    if( button == QStringLiteral( "A" ) ) {
        return SDL_CONTROLLER_BUTTON_A;
    } else if( button == QStringLiteral( "B" ) ) {
        return SDL_CONTROLLER_BUTTON_B;
    } else if( button == QStringLiteral( "X" ) ) {
        return SDL_CONTROLLER_BUTTON_X;
    } else if( button == QStringLiteral( "Y" ) ) {
        return SDL_CONTROLLER_BUTTON_Y;
    } else if( button == QStringLiteral( "Back" ) ) {
        return SDL_CONTROLLER_BUTTON_BACK;
    } else if( button == QStringLiteral( "Guide" ) ) {
        return SDL_CONTROLLER_BUTTON_GUIDE;
    } else if( button == QStringLiteral( "Start" ) ) {
        return SDL_CONTROLLER_BUTTON_START;
    } else if( button == QStringLiteral( "L3" ) ) {
        return SDL_CONTROLLER_BUTTON_LEFTSTICK;
    } else if( button == QStringLiteral( "R3" ) ) {
        return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
    } else if( button == QStringLiteral( "L" ) ) {
        return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    } else if( button == QStringLiteral( "R" ) ) {
        return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    } else if( button == QStringLiteral( "Up" ) ) {
        return SDL_CONTROLLER_BUTTON_DPAD_UP;
    } else if( button == QStringLiteral( "Down" ) ) {
        return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
    } else if( button == QStringLiteral( "Left" ) ) {
        return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
    } else if( button == QStringLiteral( "Right" ) ) {
        return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    } else {
        return SDL_CONTROLLER_BUTTON_INVALID;
    }
}
