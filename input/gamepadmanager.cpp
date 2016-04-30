#include "gamepadmanager.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QFile>

#include <memory>

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
    if( SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) < 0 ) {
        qFatal( "Fatal: Unable to initialize SDL2: %s", SDL_GetError() );
    }

    // Allow game controller event states to be automatically updated.
    SDL_GameControllerEventState( SDL_ENABLE );
}

void GamepadManager::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    switch( command ) {
        case Command::Heartbeat: {
            // Inject heartbeat into child nodes' event queues *first*
            emit commandOut( command, data, timeStamp );

            // Check connect/disconnect events
            {
                SDL_Event sdlEvent;

                // Only filter SDL_CONTROLLERDEVICEADDED and SDL_CONTROLLERDEVICEREMOVED
                while( SDL_PollEvent( &sdlEvent ) ) {
                    switch( sdlEvent.type ) {
                        case SDL_CONTROLLERDEVICEADDED: {
                            int joystickID = sdlEvent.cdevice.which;

                            Q_ASSERT( SDL_IsGameController( joystickID ) );

                            SDL_GameController *gamecontrollerHandle = SDL_GameControllerOpen( joystickID );
                            SDL_Joystick *joystickHandle = SDL_GameControllerGetJoystick( gamecontrollerHandle );
                            int instanceID = SDL_JoystickInstanceID( joystickHandle );

                            gamepads[ instanceID ].joystickID = joystickID;
                            gamepads[ instanceID ].connected = true;
                            gamepads[ instanceID ].GUID = SDL_JoystickGetGUID( joystickHandle );
                            gamepadHandles[ instanceID ] = gamecontrollerHandle;

                            qCDebug( phxInput ) << "Added controller, joystickID:" << joystickID << "instanceID:"
                                                << instanceID << "total joystickIDs:" << SDL_NumJoysticks()
                                                << "total instanceIDs:" << gamepads.size();
                            emit commandOut( Command::ControllerAdded, instanceID, QDateTime::currentMSecsSinceEpoch() );

                            break;
                        }

                        case SDL_CONTROLLERDEVICEREMOVED: {
                            int instanceID = sdlEvent.cdevice.which;
                            int joystickID = gamepads[ instanceID ].joystickID;
                            SDL_GameController *gamecontrollerHandle = SDL_GameControllerFromInstanceID( instanceID );

                            SDL_GameControllerClose( gamecontrollerHandle );
                            gamepads[ instanceID ].connected = false;

                            qCDebug( phxInput ) << "Removed controller, joystickID:" << joystickID << "instanceID:"
                                                << instanceID << "total joystickIDs:" << SDL_NumJoysticks()
                                                << "total instanceIDs:" << gamepads.size();
                            emit commandOut( Command::ControllerAdded, instanceID, QDateTime::currentMSecsSinceEpoch() );

                            break;
                        }

                        case SDL_CONTROLLERBUTTONUP:
                        case SDL_CONTROLLERBUTTONDOWN: {
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
            }

            // Emit an update for all controllers that are still connected
            {
                for( Gamepad gamepad : gamepads ) {
                    if( gamepad.connected ) {
                        mutex.lock();
                        gamepadBuffer[ gamepadBufferIndex ] = gamepad;
                        mutex.unlock();
                        emit dataOut( DataType::Input, &mutex, ( void * )( &gamepadBuffer[ gamepadBufferIndex ] ), 0, QDateTime::currentMSecsSinceEpoch() );
                        gamepadBufferIndex = ( gamepadBufferIndex + 1 ) % 100;
                    }
                }
            }

            break;
        }

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}
