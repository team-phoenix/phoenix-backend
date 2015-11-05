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
        deviceList.append( nullptr );
    }

    sdlEventLoop.start();

}

InputManager::~InputManager() {

    sdlEventLoop.stop();

    // I can't guarantee that the device won't be deleted by the deviceRemoved() signal.
    // So make sure we check.

    for( auto device : deviceList ) {
        if( device ) {
            device->selfDestruct();
        }
    }

    keyboard->selfDestruct();

}

int InputManager::count() const {
    return size();
}

int InputManager::size() const {
    return deviceList.size();
}

InputDevice *InputManager::at( int index ) {
    QMutexLocker locker( &mutex );
    return deviceList.at( index );
}

void InputManager::pollStates() {
    if( currentState == Control::PLAYING ) {
        QMutexLocker locker( &producerMutex );

        // Clear input states
        memset( inputStates, 0, sizeof( inputStates ) );

        // Fetch input states
        sdlEventLoop.pollEvents();

        for( int i = 0; i < 16; i++ ) {
            for( int j = 0; j < 16; j++ ) {
                inputStates[ i ] |= getInputState( i, RETRO_DEVICE_JOYPAD, 0, j ) << j;
            }
        }

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

            for( auto device : deviceList ) {
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

    deviceList[ joystick->port() ] = joystick;

    mutex.unlock();

    mDeviceNameMapping.insert( joystick->name(), joystick->port() );
    QQmlEngine::setObjectOwnership( joystick, QQmlEngine::CppOwnership );
    emit deviceAdded( joystick );

}

void InputManager::removeAt( int index ) {

    mutex.lock();

    auto *device = static_cast<Joystick *>( deviceList.at( index ) );
    device->selfDestruct();

    deviceList[ index ] = nullptr;

    if( deviceList.first() == nullptr ) {
        deviceList[ 0 ] = keyboard;
    }

    mutex.unlock();

}

void InputManager::swap( const int index1, const int index2 ) {
    deviceList.swap( index1, index2 );
}

void InputManager::emitConnectedDevices() {

    QQmlEngine::setObjectOwnership( keyboard, QQmlEngine::CppOwnership );
    emit deviceAdded( keyboard );

    for( int i = 0; i < deviceList.size(); ++i ) {

        auto *inputDevice = deviceList.at( i );

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

    auto port = mDeviceNameMapping.value( name, -1 );
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

int16_t InputManager::getInputState( unsigned controllerPort, unsigned retroDeviceType, unsigned analogIndex, unsigned buttonID ) {
    Q_UNUSED( analogIndex )

    // FIXME: We don't handle index for now...

    // Return nothing if there's no InputManager or no controllers connected to the given port
    if( static_cast<int>( controllerPort ) >= deviceList.size() ) {
        return 0;
    }

    // Grab the input device
    auto *inputDevice = deviceList.at( controllerPort );

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

