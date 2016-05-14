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

    // Allow game controller event states to be automatically updated.
    SDL_GameControllerEventState( SDL_ENABLE );
}

void GamepadManager::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    switch( command ) {
        case Command::Heartbeat: {
            // Check input and connect/disconnect events, update accordingly
            SDL_Event sdlEvent;

            while( SDL_PollEvent( &sdlEvent ) ) {
                switch( sdlEvent.type ) {
                    // Grab all the info we can about the controller, add this info to an entry in the gamepads hash table
                    case SDL_CONTROLLERDEVICEADDED: {
                        int joystickID = sdlEvent.cdevice.which;

                        Q_ASSERT( SDL_IsGameController( joystickID ) );

                        SDL_GameController *gamecontrollerHandle = SDL_GameControllerOpen( joystickID );
                        SDL_Joystick *joystickHandle = SDL_GameControllerGetJoystick( gamecontrollerHandle );
                        int instanceID = SDL_JoystickInstanceID( joystickHandle );
                        SDL_Haptic *haptic = SDL_HapticOpenFromJoystick( joystickHandle );

                        gamepads[ instanceID ].haptic = haptic;
                        gamepads[ instanceID ].joystickID = joystickID;
                        gamepads[ instanceID ].instanceID = instanceID;
                        gamepads[ instanceID ].GUID = SDL_JoystickGetGUID( joystickHandle );
                        gamepadHandles[ instanceID ] = gamecontrollerHandle;

                        const char *friendlyName = SDL_GameControllerName( gamecontrollerHandle );

                        if( friendlyName ) {
                            int len = static_cast<int>( strnlen( friendlyName, 1024 ) );

                            if( len ) {
                                gamepads[ instanceID ].friendlyName = QString::fromUtf8( friendlyName, len );
                            }
                        }

                        qDebug().noquote() << "Added controller:" << gamepads[ instanceID ].friendlyName
                                           << "joystickID:" << joystickID << "instanceID:"
                                           << instanceID << "Number of gamepads (SDL):" << SDL_NumJoysticks()
                                           << "Number of gamepads (Phoenix):" << gamepads.size();

                        int hapticID = -1;

                        QString hapticSupport;

                        // Set up the haptic effect and start it immediately
                        // Since the magnitude is at 0 the controller will not actually rumble until something tells it to
                        if( haptic ) {
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

                        qDebug().noquote() << "Haptic support" << ( haptic != nullptr ) << ( haptic != nullptr ? hapticSupport : QString() );
                        qDebug() << "Current configuration supported:" << SDL_HapticEffectSupported( haptic, &gamepads[ instanceID ].hapticEffect );
                        emit commandOut( Command::ControllerAdded, QVariant::fromValue( gamepads[ instanceID ] ), QDateTime::currentMSecsSinceEpoch() );
                        break;
                    }

                    // Remove the removed gamepad's entry from the gamepads hash table
                    case SDL_CONTROLLERDEVICEREMOVED: {
                        int instanceID = sdlEvent.cdevice.which;
                        int joystickID = gamepads[ instanceID ].joystickID;
                        SDL_GameController *gamecontrollerHandle = SDL_GameControllerFromInstanceID( instanceID );

                        if( gamepads[ instanceID ].haptic ) {
                            SDL_HapticStopAll( gamepads[ instanceID ].haptic );
                            SDL_HapticClose( gamepads[ instanceID ].haptic );
                        }

                        SDL_GameControllerClose( gamecontrollerHandle );

                        // All children of this node that use input (consumers of this class's input data) will
                        // mirror this change to the hash table
                        emit commandOut( Command::ControllerRemoved, QVariant::fromValue( gamepads[ instanceID ] ), QDateTime::currentMSecsSinceEpoch() );
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
            break;

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}
