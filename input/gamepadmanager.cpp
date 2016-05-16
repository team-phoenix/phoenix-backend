#include "gamepadmanager.h"

#include <QByteArray>
#include <QDateTime>
#include <QFile>

#include <memory>
#include <cstring>

#include "logging.h"

GamepadManager::GamepadManager( Node *parent ) : Node( parent ) {
    // Set the built-in mapping file
    QFile gameControllerDBFile( ":/input/gamecontrollerdb.txt" );
    bool status = gameControllerDBFile.open( QIODevice::ReadOnly );
    Q_ASSERT( status );
    auto mappingData = gameControllerDBFile.readAll();
    SDL_SetHint( SDL_HINT_GAMECONTROLLERCONFIG, mappingData.constData() );
    gameControllerDBFile.close();

    // Init SDL
    if( SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC ) < 0 ) {
        qFatal( "Fatal: Unable to initialize SDL2: %s", SDL_GetError() );
    }

    // Allow game controller and joystick event states to be included in SDL_PollEvent
    SDL_GameControllerEventState( SDL_ENABLE );
    SDL_JoystickEventState( SDL_ENABLE );
}

void GamepadManager::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    switch( command ) {
        case Command::Heartbeat: {
            // Check input and connect/disconnect events, update accordingly
            SDL_Event sdlEvent;

            while( SDL_PollEvent( &sdlEvent ) ) {
                switch( sdlEvent.type ) {
                    // Grab all the info we can about the controller, add this info to an entry in the gamepads hash table
                    case SDL_JOYDEVICEADDED: {
                        int joystickID = sdlEvent.jdevice.which;

                        SDL_Joystick *joystickHandle = SDL_JoystickOpen( joystickID );
                        int instanceID = SDL_JoystickInstanceID( joystickHandle );

                        GamepadState &gamepad = gamepads[ instanceID ];
                        gamepad.joystickID = joystickID;
                        gamepad.instanceID = instanceID;
                        gamepad.joystickHandle = joystickHandle;
                        gamepad.GUID = SDL_JoystickGetGUID( joystickHandle );
                        const char *friendlyName = SDL_JoystickName( joystickHandle );

                        if( friendlyName ) {
                            gamepad.friendlyName = QString::fromUtf8( friendlyName );
                        }

                        // Sanity check on the number of available axes, buttons and joysticks
                        {
                            // TODO: Support more than 16 axes?
                            if( SDL_JoystickNumAxes( joystickHandle ) > 16 ) {
                                qWarning() << "Ignoring controller with more than 16 axes, GUID ="
                                           << QByteArray( reinterpret_cast<const char *>( gamepad.GUID.data ), 16 );
                                SDL_JoystickClose( joystickHandle );
                                break;
                            }

                            // TODO: Support more than 16 hats?
                            if( SDL_JoystickNumHats( joystickHandle ) > 16 ) {
                                qWarning() << "Ignoring controller with more than 16 hats, GUID ="
                                           << QByteArray( reinterpret_cast<const char *>( gamepad.GUID.data ), 16 );
                                SDL_JoystickClose( joystickHandle );
                                break;
                            }

                            // TODO: Support more than 256 buttons?
                            if( SDL_JoystickNumButtons( joystickHandle ) > 256 ) {
                                qWarning() << "Ignoring controller with more than 256 buttons, GUID ="
                                           << QByteArray( reinterpret_cast<const char *>( gamepad.GUID.data ), 16 );
                                SDL_JoystickClose( joystickHandle );
                                break;
                            }
                        }

                        // Set up the joystick as a game controller if a mapping is available
                        {
                            // If this is a game controller, reopen as one and grab some more info
                            SDL_GameController *gamecontrollerHandle = nullptr;

                            if( SDL_IsGameController( joystickID ) ) {
                                SDL_JoystickClose( joystickHandle );
                                gamecontrollerHandle = SDL_GameControllerOpen( joystickID );
                                joystickHandle = SDL_GameControllerGetJoystick( gamecontrollerHandle );
                                char *mappingString = SDL_GameControllerMapping( gamecontrollerHandle );
                                gamepad.mappingString = QString::fromUtf8( mappingString );
                                SDL_free( mappingString );
                                const char *friendlyName = SDL_GameControllerName( gamecontrollerHandle );

                                if( friendlyName ) {
                                    gamepad.friendlyName = QString::fromUtf8( friendlyName );
                                }

                                gamepad.gamecontrollerHandle = gamecontrollerHandle;
                                gamepad.joystickHandle = joystickHandle;
                            }
                        }

                        // Print some info about the controller
                        {
                            qDebug().noquote() << "Added controller:" << gamepad.friendlyName
                                               << "joystickID:" << joystickID << "instanceID:"
                                               << instanceID << "Number of gamepads (SDL):" << SDL_NumJoysticks()
                                               << "Number of gamepads (Phoenix):" << gamepads.size();

                            if( !SDL_IsGameController( joystickID ) ) {
                                qInfo() << "Device has no mapping yet";
                            }
                        }

                        // Set up haptic (rumble) support
                        {
                            int hapticID = -1;

                            SDL_Haptic *haptic = SDL_HapticOpenFromJoystick( joystickHandle );
                            gamepad.haptic = haptic;
                            QString hapticSupport;

                            // Set up the haptic effect and start it immediately
                            // Since the magnitude is at 0 the controller will not actually rumble until something tells it to
                            if( haptic && SDL_HapticQuery( haptic ) ) {
                                gamepads[ instanceID ].hapticEffect.type = SDL_HAPTIC_LEFTRIGHT;
                                gamepads[ instanceID ].hapticEffect.leftright.length = SDL_HAPTIC_INFINITY;
                                hapticID = SDL_HapticNewEffect( gamepads[ instanceID ].haptic, &gamepads[ instanceID ].hapticEffect );

                                // If either effect set up correctly, start it now
                                if( hapticID >= 0 ) {

                                    // Start the effect now
                                    if( SDL_HapticRunEffect( haptic, hapticID, 1 ) < 0 ) {
                                        qWarning() << "SDL_HapticRunEffect failed on" << gamepads[ instanceID ].friendlyName << hapticID << SDL_GetError();
                                    } else {
                                        qDebug() << "Started haptic effect successfully, hapticID:" << hapticID;

                                        // Set the effect ID for later use
                                        gamepads[ instanceID ].hapticID = hapticID;
                                    }
                                } else {
                                    qWarning() << "SDL_HapticNewEffect failed on" << gamepads[ instanceID ].friendlyName << hapticID << SDL_GetError();
                                }

                                // Build a string to dump to console containing info about all possible types and their support
                                hapticSupport = QString(
                                                    "SDL_HAPTIC_CONSTANT: %1 "
                                                    "SDL_HAPTIC_SINE: %2 "
                                                    "SDL_HAPTIC_TRIANGLE: %3 "
                                                    "SDL_HAPTIC_SAWTOOTHUP: %4 "
                                                    "SDL_HAPTIC_SAWTOOTHDOWN: %5 "
                                                    "SDL_HAPTIC_SPRING: %6 "
                                                    "SDL_HAPTIC_DAMPER: %7 "
                                                    "SDL_HAPTIC_INERTIA: %8 "
                                                    "SDL_HAPTIC_FRICTION: %9 "
                                                    "SDL_HAPTIC_RAMP: %10 "
                                                    "SDL_HAPTIC_LEFTRIGHT: %11 "
                                                    "SDL_HAPTIC_CUSTOM: %12 "
                                                )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_CONSTANT ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_SINE ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_TRIANGLE ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_SAWTOOTHUP ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_SAWTOOTHDOWN ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_SPRING ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_DAMPER ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_INERTIA ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_FRICTION ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_RAMP ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_LEFTRIGHT ) != 0 )
                                                .arg( ( SDL_HapticQuery( haptic ) & SDL_HAPTIC_CUSTOM ) != 0 );
                            }

                            qDebug().noquote() << "Haptic (rumble) support" << ( haptic != nullptr ) << ( haptic != nullptr ? hapticSupport : QString() );
                            qDebug() << "Current configuration supported:" << SDL_HapticEffectSupported( haptic, &gamepad.hapticEffect );
                        }

                        // Inform the consumers a new controller was just added
                        emit commandOut( Command::ControllerAdded, QVariant::fromValue( gamepad ), QDateTime::currentMSecsSinceEpoch() );

                        break;
                    }

                    // Remove the removed gamepad's entry from the gamepads hash table
                    case SDL_JOYDEVICEREMOVED: {
                        int instanceID = sdlEvent.jdevice.which;
                        GamepadState &gamepad = gamepads[ instanceID ];

                        int joystickID = gamepad.joystickID;
                        SDL_GameController *gamecontrollerHandle = gamepad.gamecontrollerHandle;
                        SDL_Joystick *joystickHandle = gamepad.joystickHandle;

                        // Shut down controller-related SDL structures
                        {
                            if( gamepads[ instanceID ].haptic ) {
                                SDL_HapticStopAll( gamepads[ instanceID ].haptic );
                                SDL_HapticClose( gamepads[ instanceID ].haptic );
                            }

                            if( SDL_IsGameController( joystickID ) ) {
                                SDL_GameControllerClose( gamecontrollerHandle );
                            } else {
                                SDL_JoystickClose( joystickHandle );
                            }
                        }

                        // Inform the consumers this controller was removed so it can be deleted from their lists or otherwise marked as unplugged
                        emit commandOut( Command::ControllerRemoved, QVariant::fromValue( gamepads[ instanceID ] ), QDateTime::currentMSecsSinceEpoch() );

                        // Remove it here, too
                        gamepads.remove( instanceID );

                        qCDebug( phxInput ) << "Removed controller, joystickID:" << joystickID << "instanceID:"
                                            << instanceID << "Number of gamepads (SDL):" << SDL_NumJoysticks()
                                            << "Number of gamepads (Phoenix):" << gamepads.size();
                        break;
                    }

                    // Update our button state
                    case SDL_CONTROLLERBUTTONDOWN:
                    case SDL_CONTROLLERBUTTONUP: {
                        SDL_JoystickID instanceID = sdlEvent.cbutton.which;
                        Uint8 buttonID = sdlEvent.cbutton.button;
                        Uint8 state = sdlEvent.cbutton.state;
                        gamepads[ instanceID ].button[ buttonID ] = state;
                        break;
                    }

                    case SDL_CONTROLLERAXISMOTION: {
                        SDL_JoystickID instanceID = sdlEvent.cbutton.which;
                        Uint8 axis = sdlEvent.caxis.axis;
                        Sint16 value = sdlEvent.caxis.value;
                        gamepads[ instanceID ].axis[ axis ] = value;
                        break;
                    }

                    case SDL_JOYAXISMOTION: {
                        break;
                    }

                    case SDL_JOYHATMOTION: {
                        break;
                    }

                    case SDL_JOYBUTTONDOWN:
                    case SDL_JOYBUTTONUP: {
                        break;
                    }

                    default: {
                        break;
                    }
                }
            }

            // Emit an update for all connected controllers
            for( GamepadState gamepad : gamepads ) {
                // Copy current gamepad into buffer
                mutex.lock();
                gamepadBuffer[ gamepadBufferIndex ] = gamepad;
                mutex.unlock();

                // Send buffer on its way
                emit dataOut( DataType::Input, &mutex, &gamepadBuffer[ gamepadBufferIndex ], 0, QDateTime::currentMSecsSinceEpoch() );

                // Increment the index
                gamepadBufferIndex = ( gamepadBufferIndex + 1 ) % 100;
            }

            // Inject heartbeat into child nodes' event queues *after* input data
            emit commandOut( command, data, timeStamp );

            break;
        }

        case Command::SetUserDataPath:
            userDataPath = data.toString();
            qDebug() << "Using path" << userDataPath;
            break;

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}
