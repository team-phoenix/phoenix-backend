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

                        gamepads[ instanceID ].joystickID = joystickID;
                        gamepads[ instanceID ].instanceID = instanceID;
                        gamepads[ instanceID ].GUID = SDL_JoystickGetGUID( joystickHandle );
                        gamepadHandles[ instanceID ] = gamecontrollerHandle;

                        qCDebug( phxInput ) << "Added controller, joystickID:" << joystickID << "instanceID:"
                                            << instanceID << "total joystickIDs:" << SDL_NumJoysticks()
                                            << "total instanceIDs:" << gamepads.size();
                        emit commandOut( Command::ControllerAdded, instanceID, QDateTime::currentMSecsSinceEpoch() );

                        break;
                    }

                    // Remove the removed gamepad's entry from the gamepads hash table
                    case SDL_CONTROLLERDEVICEREMOVED: {
                        int instanceID = sdlEvent.cdevice.which;
                        int joystickID = gamepads[ instanceID ].joystickID;
                        SDL_GameController *gamecontrollerHandle = SDL_GameControllerFromInstanceID( instanceID );

                        SDL_GameControllerClose( gamecontrollerHandle );
                        gamepads.remove( instanceID );

                        qCDebug( phxInput ) << "Removed controller, joystickID:" << joystickID << "instanceID:"
                                            << instanceID << "total joystickIDs:" << SDL_NumJoysticks()
                                            << "total instanceIDs:" << gamepads.size();

                        // All children of this node that use input (consumers of this class's input data) will
                        // mirror this change to the hash table
                        emit commandOut( Command::ControllerRemoved, instanceID, QDateTime::currentMSecsSinceEpoch() );

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
            for( Gamepad gamepad : gamepads ) {
                // Copy current gamepad into buffer
                mutex.lock();
                gamepadBuffer[ gamepadBufferIndex ] = gamepad;
                mutex.unlock();

                // Send buffer on its way
                emit dataOut( DataType::Input, &mutex, ( void * )( &gamepadBuffer[ gamepadBufferIndex ] ), 0, QDateTime::currentMSecsSinceEpoch() );

                // Increment the index
                gamepadBufferIndex = ( gamepadBufferIndex + 1 ) % 100;
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
