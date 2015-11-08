#include "inputmanager.h"

InputManager::InputManager( QObject *parent )
    : QObject( parent ),
      keyboard( new Keyboard() ),
      sdlEventLoop( this ),
      inputStates{ 0 } {

    keyboard->loadMapping();

    connect( &sdlEventLoop, &SDLEventLoop::deviceConnected, this, &InputManager::insert );
    connect( &sdlEventLoop, &SDLEventLoop::deviceRemoved, this, &InputManager::removeAt );

    connect( this, &InputManager::controllerDBFileChanged, &sdlEventLoop, &SDLEventLoop::onControllerDBFileChanged );

    // The Keyboard will be always active in port 0,
    // unless changed by the user.

    for( int i = 0; i < Joystick::maxNumOfDevices; ++i ) {
        gamepadList.append( nullptr );
    }

    sdlEventLoop.start();

}

InputManager::~InputManager() {

    sdlEventLoop.stop();

    // I can't guarantee that the device won't be deleted by the deviceRemoved() signal.
    // So make sure we check.

    for( auto device : gamepadList ) {
        if( device ) {
            device->selfDestruct();
        }
    }

    keyboard->selfDestruct();

}

int InputManager::size() const {
    return gamepadList.size();
}

InputDevice *InputManager::at( int index ) {
    QMutexLocker locker( &mutex );
    return gamepadList.at( index );
}

void InputManager::libretroGetInputState() {
    if( currentState == Control::PLAYING ) {
        producerMutex.lock();

        // Clear input states
        memset( inputStates, 0, sizeof( inputStates ) );

        // Fetch input states
        sdlEventLoop.pollEvents();

        for( int i = 0; i < 16; i++ ) {
            for( int j = 0; j < 16; j++ ) {
                inputStates[ i ] |= libretroGetInputStateHelper( i, RETRO_DEVICE_JOYPAD, 0, j ) << j;
            }
        }

        producerMutex.unlock();

        emit producerData( QStringLiteral( "input" ), &producerMutex, inputStates, sizeof( inputStates ), QDateTime::currentMSecsSinceEpoch() );
    }
}

bool InputManager::gamepadControlsFrontend() const {
    return InputDevice::gamepadControlsFrontend;
}

void InputManager::setGamepadControlsFrontend( const bool control ) {
    InputDevice::gamepadControlsFrontend = control;
    emit gamepadControlsFrontendChanged();
}

void InputManager::registerTypes() {
    qmlRegisterType<InputManager>( "vg.phoenix.backend", 1, 0, "InputManager" );
    qmlRegisterType<InputDeviceEvent>( "vg.phoenix.backend", 1, 0, "InputDeviceEvent" );
    qmlRegisterType<QMLInputDevice>( "vg.phoenix.backend", 1, 0, "QMLInputDevice" );

    qRegisterMetaType<InputDevice *>( "InputDevice *" );
}

void InputManager::setState( Control::State state ) {

    // We only care about the transition to or away from PLAYING
    if( ( this->currentState == Control::PLAYING && state != Control::PLAYING ) ||
        ( this->currentState != Control::PLAYING && state == Control::PLAYING ) ) {
        QMutexLocker locker( &mutex );
        bool run = ( state == Control::PLAYING );

        setGamepadControlsFrontend( !run );

        if( run ) {
            qCDebug( phxInput ) << "Reading game input from keyboard";
            sdlEventLoop.stop();
            installKeyboardFilter();

            for( auto device : gamepadList ) {
                if( device ) {
                    device->setEditMode( false );
                }
            }
        }

        else {
            qCDebug( phxInput ) << "No longer reading keyboard input";
            sdlEventLoop.start();
            removeKeyboardFilter();
        }
    }

    this->currentState = state;

}

void InputManager::insert( InputDevice *device ) {

    device->loadMapping();
    mutex.lock();
    auto *joystick = device;

    gamepadList[ joystick->port() ] = joystick;

    mutex.unlock();

    gamepadNameMapping.insert( joystick->name(), joystick->port() );
    QQmlEngine::setObjectOwnership( joystick, QQmlEngine::CppOwnership );
    emit deviceAdded( joystick );

}

void InputManager::removeAt( int index ) {

    mutex.lock();

    auto *device = static_cast<Joystick *>( gamepadList.at( index ) );
    device->selfDestruct();

    gamepadList[ index ] = nullptr;

    if( gamepadList.first() == nullptr ) {
        gamepadList[ 0 ] = keyboard;
    }

    mutex.unlock();

}

void InputManager::swap( const int index1, const int index2 ) {
    gamepadList.swap( index1, index2 );
}

void InputManager::emitConnectedDevices() {

    QQmlEngine::setObjectOwnership( keyboard, QQmlEngine::CppOwnership );
    emit deviceAdded( keyboard );

    for( int i = 0; i < gamepadList.size(); ++i ) {

        auto *inputDevice = gamepadList.at( i );

        if( inputDevice ) {
            QQmlEngine::setObjectOwnership( inputDevice, QQmlEngine::CppOwnership );
            emit deviceAdded( inputDevice );
        }

    }

}

InputDevice *InputManager::get( const QString name ) {
    if( keyboard->name() == name ) {
        return keyboard;
    }

    auto port = gamepadNameMapping.value( name, -1 );
    Q_ASSERT( port != -1 );
    return at( port );
}

bool InputManager::eventFilter( QObject *object, QEvent *event ) {

    if( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease ) {
        auto *keyEvent = static_cast<QKeyEvent *>( event );
        keyboard->insert( keyEvent->key(), event->type() == QEvent::KeyPress );
        keyEvent->accept();
        return true;
    }

    return QObject::eventFilter( object, event );

}

void InputManager::installKeyboardFilter() {
    Q_ASSERT( QGuiApplication::topLevelWindows().size() > 0 );

    auto *window =  QGuiApplication::topLevelWindows().at( 0 );

    Q_CHECK_PTR( window );

    window->installEventFilter( this );
}

void InputManager::removeKeyboardFilter() {
    Q_ASSERT( QGuiApplication::topLevelWindows().size() > 0 );
    auto *window =  QGuiApplication::topLevelWindows().at( 0 );

    Q_CHECK_PTR( window );
    window->removeEventFilter( this );
}

int16_t InputManager::libretroGetInputStateHelper( unsigned controllerPort, unsigned retroDeviceType, unsigned analogIndex, unsigned buttonID ) {
    Q_UNUSED( analogIndex )

    // FIXME: We don't handle index for now...

    // Return nothing if there's no InputManager or no controllers connected to the given port
    if( static_cast<int>( controllerPort ) >= gamepadList.size() ) {
        return 0;
    }

    // Grab the input device
    auto *inputDevice = gamepadList.at( controllerPort );

    auto event = static_cast<InputDeviceEvent::Event>( buttonID );


    if( controllerPort == 0 ) {
        auto keyState = keyboard->value( event, 0 );

        if( !inputDevice ) {
            return keyState;
        }

        auto deviceState = inputDevice->value( event, 0 );

        return deviceState | keyState;
    }

    // make sure the InputDevice was configured
    // to map to the requested RETRO_DEVICE.

    if( !inputDevice || inputDevice->type() != static_cast<InputDevice::LibretroType>( retroDeviceType ) ) {
        return 0;
    }

    return inputDevice->value( event, 0 );

}

