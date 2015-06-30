#include "sdleventloop.h"

#include "logging.h"

#include <QFile>
#include <QMutexLocker>


SDLEventLoop::SDLEventLoop( QObject *parent )
    : QObject( parent ),
      sdlPollTimer( this ),
      numOfDevices( 0 ),
      forceEventsHandling( true ) {

    // TODO: The poll timer isn't in the sdlEventLoopThread. It needs to be.

    // Ensures the resources at loaded at startup, even during
    // static compilation.
    Q_INIT_RESOURCE( controllerdb );
    QFile gameControllerDBFile( ":/input/gamecontrollerdb.txt" );
    Q_ASSERT( gameControllerDBFile.open( QIODevice::ReadOnly ) );

    auto mappingData = gameControllerDBFile.readAll();

    SDL_SetHint( SDL_HINT_GAMECONTROLLERCONFIG, mappingData.constData() );

    gameControllerDBFile.close();

    for( int i = 0; i < Joystick::maxNumOfDevices; ++i ) {
        sdlDeviceList.append( nullptr );
    }

    sdlPollTimer.setInterval( 5 );

    connect( &sdlPollTimer, &QTimer::timeout, this, &SDLEventLoop::pollEvents );

    // Load SDL
    initSDL();

}

void SDLEventLoop::pollEvents() {

    if( !forceEventsHandling ) {

        // Update all connected controller states.
        SDL_GameControllerUpdate();

        // All joystick instance ID's are stored inside of this map.
        // This is necessary because the instance ID could be any number, and
        // so cannot be used for indexing the deviceLocationMap. The value of the map
        // is the actual index that the sdlDeviceList uses.
        for( auto &key : deviceLocationMap.keys() ) {

            auto index = deviceLocationMap[ key ];

            auto *joystick = sdlDeviceList.at( index );
            auto *sdlGamepad = joystick->sdlDevice();

            // Check to see if sdlGamepad is actually connected. If it isn't this will terminate the
            // polling and initialize the event handling.

            forceEventsHandling = joystick->editMode()
                                  | ( SDL_GameControllerGetAttached( sdlGamepad ) == SDL_FALSE );

            if( forceEventsHandling ) {
                return;
            }

            bool left, right, down, up, a, b, x, y, start, select, rightShoulder, leftShoulder;
            bool guide, leftStick, rightStick;

            qint16 leftTrigger, rightTrigger, leftXAxis, leftYAxis, rightXAxis, rightYAxis;
            Q_UNUSED( rightXAxis );
            Q_UNUSED( rightYAxis );

            // Read D-PAD Button States
            left = joystick->getButtonState( SDL_CONTROLLER_BUTTON_DPAD_LEFT );
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

            joystick->insert( InputDeviceEvent::Left, left );
            joystick->insert( InputDeviceEvent::Right, right );
            joystick->insert( InputDeviceEvent::Down, down );
            joystick->insert( InputDeviceEvent::Up,  up );

            joystick->insert( InputDeviceEvent::Start, start );
            joystick->insert( InputDeviceEvent::Select, select );

            // The guide button is emitted to the frontend and is hooked up the to
            // QMLInputDevice.
            joystick->emitInputDeviceEvent( InputDeviceEvent::Guide, guide );

            // The buttons are switched to a SNES controller layout.
            // SDL GameControllers have Xbox360 controller layouts.
            joystick->insert( InputDeviceEvent::A, b );
            joystick->insert( InputDeviceEvent::B, a );
            joystick->insert( InputDeviceEvent::X, y );
            joystick->insert( InputDeviceEvent::Y, x );

            joystick->insert( InputDeviceEvent::L3, leftStick );
            joystick->insert( InputDeviceEvent::R3, rightStick );

            joystick->insert( InputDeviceEvent::L, leftShoulder );
            joystick->insert( InputDeviceEvent::R, rightShoulder );

            joystick->insert( InputDeviceEvent::L2, leftTrigger );
            joystick->insert( InputDeviceEvent::R2, rightTrigger );

            //qDebug() << left << right << down << up << start << select <<
            //         a << b << x << y << leftShoulder << rightShoulder << leftTrigger << rightTrigger
            //     << leftStick << rightStick << leftXAxis << leftYAxis << rightYAxis << rightXAxis;

        }

    }

    else {

        SDL_Event sdlEvent;

        // The only events that should be handled here are, SDL_CONTROLLERDEVICEADDED
        // and SDL_CONTROLLERDEVICEREMOVED.
        while( SDL_PollEvent( &sdlEvent ) ) {

            switch( sdlEvent.type ) {

                case SDL_CONTROLLERDEVICEADDED: {

                    forceEventsHandling = false;

                    // This needs to be checked for, because the first time a controller
                    // sdl starts up, it fires this signal twice, pretty annoying...

                    if( sdlDeviceList.at( sdlEvent.cdevice.which ) != nullptr ) {

                        qCDebug( phxInput ).nospace() << "Duplicate controller added at slot "
                                                      << sdlEvent.cdevice.which << ", ignored";
                        break;

                    }

                    auto *joystick = new Joystick( sdlEvent.cdevice.which );

                    deviceLocationMap.insert( joystick->instanceID(), sdlEvent.cdevice.which );

                    sdlDeviceList[ sdlEvent.cdevice.which ] = joystick;

                    emit deviceConnected( joystick );

                    break;

                }

                case SDL_CONTROLLERDEVICEREMOVED: {

                    int index = deviceLocationMap.value( sdlEvent.cbutton.which, -1 );

                    Q_ASSERT( index != -1 );

                    auto *joystick = sdlDeviceList.at( index );

                    Q_ASSERT( joystick != nullptr );

                    if( joystick->instanceID() == sdlEvent.cdevice.which ) {

                        emit deviceRemoved( joystick->sdlIndex() );
                        sdlDeviceList[ index ] = nullptr;
                        deviceLocationMap.remove( sdlEvent.cbutton.which );
                        forceEventsHandling = true;
                        break;

                    }

                    break;

                }

                case SDL_CONTROLLERBUTTONUP:
                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP: {

                    int index = deviceLocationMap.value( sdlEvent.cbutton.which, -1 );
                    Q_ASSERT( index != -1 );

                    auto *joystick = sdlDeviceList.at( index );

                    Q_ASSERT( joystick != nullptr );

                    int state = sdlEvent.cbutton.state;

                    joystick->emitEditModeEvent( sdlEvent.cbutton.button, state );

                    break;

                }

                default:
                    break;

            }

        }

    }

}

void SDLEventLoop::start() {
    sdlPollTimer.start();
}

void SDLEventLoop::stop() {
    sdlPollTimer.stop();
}

void SDLEventLoop::initSDL() {

    if( SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) < 0 ) {
        qFatal( "Fatal: Unable to initialize SDL2: %s", SDL_GetError() );
    }

    // Allow game controller event states to be automatically updated.
    SDL_GameControllerEventState( SDL_ENABLE );

}

void SDLEventLoop::quitSDL() {
    SDL_Quit();
}
