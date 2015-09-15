#include "inputmanager.h"

#include "qmlinputdevice.h"
#include <qqml.h>

#include <QApplication>
#include <QWindow>
#include <QQmlEngine>

InputManager::InputManager( QObject *parent )
    : QObject( parent ),
      keyboard( new Keyboard() ),
      sdlEventLoop( this ) {

    keyboard->loadMapping();

    connect( &sdlEventLoop, &SDLEventLoop::deviceConnected, this, &InputManager::insert );
    connect( &sdlEventLoop, &SDLEventLoop::deviceRemoved, this, &InputManager::removeAt );

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
    sdlEventLoop.pollEvents();

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

void InputManager::setRun( bool run ) {

    mutex.lock();

    setGamepadControlsFrontend( !run );

    if( run ) {
        sdlEventLoop.stop();
        installKeyboardFilter();

        for( auto device : deviceList ) {
            if( device ) {
                device->setEditMode( false );
            }
        }
    }

    else {
        sdlEventLoop.start();
        removeKeyboardFilter();
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

void InputManager::installKeyboardFilter()
{
    Q_ASSERT( QApplication::topLevelWindows().size() > 0 );

    auto *window =  QApplication::topLevelWindows().at( 0 );

    Q_ASSERT( window );

    window->installEventFilter( this );
}

void InputManager::removeKeyboardFilter()
{
    Q_ASSERT( QApplication::topLevelWindows().size() > 0 );
    auto *window =  QApplication::topLevelWindows().at( 0 );

    Q_ASSERT( window );
    window->removeEventFilter( this );
}

