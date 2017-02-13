#include "inputmanager.h"
#include "logging.h"

#include <SDL_gamecontroller.h>
#include <SDL_haptic.h>
#include <SDL_joystick.h>

#include <SDL.h>

#include <QCoreApplication>
#include <QThread>

InputManager::InputManager(QObject *parent)
    : QObject( parent )
{
    // Init SDL
    if( SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC ) < 0 ) {
        qFatal( "Fatal: Unable to initialize SDL2: %s", SDL_GetError() );
    }

    // Allow game controller and joystick event states to be included in SDL_PollEvent
    SDL_GameControllerEventState( SDL_ENABLE );
    SDL_JoystickEventState( SDL_ENABLE );

    qCDebug( phxInput ) << "SDL2 initialized";

    while( true ) {
        poll();

        QCoreApplication::processEvents();

        QThread::msleep( 5 );
    }
}

void InputManager::poll() {
    //qCDebug( phxInput ) << "polling for input";

    SDL_Event sdlEvent;

    while( SDL_PollEvent( &sdlEvent ) ) {
        switch( sdlEvent.type ) {
            case SDL_JOYDEVICEADDED: {
                int joystickID = sdlEvent.jdevice.which;
                SDL_Joystick *joystickHandle = SDL_JoystickOpen( joystickID );
                int instanceID = SDL_JoystickInstanceID( joystickHandle );
                const char *friendlyName = SDL_JoystickName( joystickHandle );
                if ( !friendlyName ) {
                    friendlyName = "Unknown";
                }

                qCDebug( phxInput ) << "Added" << friendlyName;
                break;
            }
        case SDL_JOYDEVICEREMOVED: {
            qCDebug( phxInput ) << "device removed";
            break;
        }
            default: {
                qCDebug( phxInput ) << sdlEvent.type;
                break;
            }
        }
    }
}
