#include "sdleventloop.h"
#include "logging.h"
#include "joystickevent.h"

#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>
#include <SDL.h>

#include <QFile>
#include <QRegularExpression>

#include <QCoreApplication>

SDLEventLoop::SDLEventLoop( QObject *parent )
    : QObject( parent )
{
    // Init controller db file
    //Q_INIT_RESOURCE( controllerdb );

    // TODO: The poll timer isn't in the sdlEventLoopThread. It needs to be.

    QFile gameControllerDBFile( ":/input/gamecontrollerdb.txt" );
    bool status = gameControllerDBFile.open( QIODevice::ReadOnly );
    Q_ASSERT( status );

    init( gameControllerDBFile.readAll() );

}

SDLEventLoop::~SDLEventLoop() {
    deinit();
}

void SDLEventLoop::poll( qint64 timestamp ) {
    Q_UNUSED( timestamp );
    SDL_GameControllerUpdate();
    auto numOfJoysticks = SDL_NumJoysticks();

    // Add joysticks
    if ( numOfJoysticks > m_gamepadsMap.size() ) {
        for ( int i=0; i < numOfJoysticks; ++i ) {
            if ( SDL_IsGameController( i ) ) {
                auto *gamepad = new Gamepad( i );
                qCDebug( phxInput, "Retropad(%d) added", gamepad->id() );
                m_gamepadsMap[ gamepad->id() ] = gamepad;
                emit gamepadAdded( gamepad );
            }
        }

        // Remove joysticks
    } else if ( numOfJoysticks < m_gamepadsMap.size() ) {
        for ( auto id : m_gamepadsMap.keys() ) {
            auto *gamepad = m_gamepadsMap[ id ];
            if ( !gamepad->isOpen() ) {
                qCDebug( phxInput, "Retropad(%d) removed", gamepad->id() );
                emit gamepadRemoved( gamepad );
                gamepad->deleteLater();
                m_gamepadsMap.remove( id );
            }
        }
        // Update joystick states for all controllers.
    } else {

        for ( auto id : m_gamepadsMap.keys() ) {
            m_gamepadsMap[ id ]->update();
        }

    }

        // Update all connected controller states.

            //quint8 left, right, down, up, a, b, x, y, start, select, rightShoulder, leftShoulder;
           // quint8 guide, leftStick, rightStick;

           // qint16 leftTrigger, rightTrigger, leftXAxis, leftYAxis, rightXAxis, rightYAxis;
            //Q_UNUSED( rightXAxis );
           // Q_UNUSED( rightYAxis );

            /*

            right = joystick->getButtonState( SDL_CONTROLLER_BUTTON_DPAD_RIGHT );
            up = joystick->getButtonState( SDL_CONTROLLER_BUTTON_DPAD_UP );
            down = joystick->getButtonState( SDL_CONTROLLER_BUTTON_DPAD_DOWN );

            // Read Menu Button States
            start = joystick->getButtonState( SDL_CONTROLLER_BUTTON_START );
            select = joystick->getButtonState( SDL_CONTROLLER_BUTTON_BACK );
            guide = joystick->getButtonState( SDL_CONTROLLER_BUTTON_GUIDE );

            // Read Action Button States
            a = joystick->getButtonState( SDL_CONTROLLER_BUTTON_A );
            b = joystick->getButtonState( SDL_CONTROLLER_BUTTON_B );
            x = joystick->getButtonState( SDL_CONTROLLER_BUTTON_X );
            y = joystick->getButtonState( SDL_CONTROLLER_BUTTON_Y );

            // Read Analog Click Button States
            leftStick = joystick->getButtonState( SDL_CONTROLLER_BUTTON_LEFTSTICK );
            rightStick = joystick->getButtonState( SDL_CONTROLLER_BUTTON_RIGHTSTICK );

            // Read Shoulder Button States
            leftShoulder = joystick->getButtonState( SDL_CONTROLLER_BUTTON_LEFTSHOULDER );
            rightShoulder = joystick->getButtonState( SDL_CONTROLLER_BUTTON_RIGHTSHOULDER );

            // Check if the joystick has digital triggers, like the Wii U Pro Controlle, and
            // read the values.

            leftTrigger = joystick->getAxisState( SDL_CONTROLLER_AXIS_TRIGGERLEFT );

            rightTrigger = joystick->getAxisState( SDL_CONTROLLER_AXIS_TRIGGERRIGHT );

            //gamecontroller->mapping.buttons[button]
            // Read Analog Joystick Values
            leftXAxis = joystick->getAxisState( SDL_CONTROLLER_AXIS_LEFTX );
            leftYAxis = joystick->getAxisState( SDL_CONTROLLER_AXIS_LEFTY );
            rightXAxis = joystick->getAxisState( SDL_CONTROLLER_AXIS_RIGHTX );
            rightYAxis = joystick->getAxisState( SDL_CONTROLLER_AXIS_RIGHTY );

            // !analogMode means that the console being played doesn't support
            // analog sticks. We will then have the left analog stick mimic the D-PAD.
            if( !joystick->analogMode() ) {

                if( leftXAxis <= 0 ) {
                    left |= ( leftXAxis < -joystick->deadZone() );
                }

                if( leftXAxis > 0 ) {
                    right |= ( leftXAxis > joystick->deadZone() );
                }

                if( leftYAxis <= 0 ) {
                    up |= ( leftYAxis < -joystick->deadZone() );
                }

                if( leftYAxis > 0 ) {
                    down |= ( leftYAxis > joystick->deadZone() );
                }

            }



            joystick->insert( JoystickButtonEvent::Left, left );
            joystick->insert( JoystickButtonEvent::Right, right );
            joystick->insert( JoystickButtonEvent::Down, down );
            joystick->insert( JoystickButtonEvent::Up,  up );

            joystick->insert( JoystickButtonEvent::Start, start );
            joystick->insert( JoystickButtonEvent::Select, select );

            // The guide button is emitted to the frontend and is hooked up the to
            // QMLInputDevice.
            joystick->emitInputDeviceEvent( JoystickButtonEvent::Guide, guide );

            // The buttons are switched to a SNES controller layout.
            // SDL GameControllers have Xbox360 controller layouts.
            joystick->insert( JoystickButtonEvent::A, b );
            joystick->insert( JoystickButtonEvent::B, a );
            joystick->insert( JoystickButtonEvent::X, y );
            joystick->insert( JoystickButtonEvent::Y, x );

            joystick->insert( JoystickButtonEvent::L3, leftStick );
            joystick->insert( JoystickButtonEvent::R3, rightStick );

            joystick->insert( JoystickButtonEvent::L, leftShoulder );
            joystick->insert( JoystickButtonEvent::R, rightShoulder );

            joystick->insert( JoystickButtonEvent::L2, leftTrigger );
            joystick->insert( JoystickButtonEvent::R2, rightTrigger );

            //qDebug() << left << right << down << up << start << select <<
            //         a << b << x << y << leftShoulder << rightShoulder << leftTrigger << rightTrigger
            //     << leftStick << rightStick << leftXAxis << leftYAxis << rightYAxis << rightXAxis;
*/
}

void SDLEventLoop::onControllerDBFileChanged( QString controllerDBFile ) {
    qCDebug( phxInput ) << "Opening custom controller DB file:" << qPrintable( controllerDBFile.remove( QRegularExpression( "[\"]." ) ) );

    QFile gameControllerDBFile( controllerDBFile );

    if( !gameControllerDBFile.open( QIODevice::ReadOnly ) ) {
        qCDebug( phxInput ) << "Custom controller DB file not present, using default file only";
        return;
    }

    // We're good to go, load the custom file
    deinit();

    auto mappingData = gameControllerDBFile.readAll();

    SDL_SetHint( SDL_HINT_GAMECONTROLLERCONFIG, mappingData.constData() );

    qCDebug( phxInput ) << "Loaded custom controller DB successfully.";

    // Use the default one, too
    QFile gameControllerEmbeddedDBFile( ":/input/gamecontrollerdb.txt" );
    bool status = gameControllerEmbeddedDBFile.open( QIODevice::ReadOnly );
    Q_ASSERT( status );

    init( gameControllerEmbeddedDBFile.readAll() );

}

void SDLEventLoop::init( const QByteArray &mapData ) {
    SDL_SetHint( SDL_HINT_GAMECONTROLLERCONFIG, mapData.constData() );
    if( SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) < 0 ) {
        qFatal( "Fatal: Unable to initialize SDL2: %s", SDL_GetError() );
    }
}

void SDLEventLoop::deinit() {
    SDL_Quit();
}
