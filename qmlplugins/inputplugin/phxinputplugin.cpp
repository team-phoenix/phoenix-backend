#include "phxinputplugin.h"

#include "inputmanager.h"

void PhxInputPlugin::registerTypes( const char *uri ) {
    qmlRegisterType<InputManager>( uri, 1, 0, "PhxInputManager" );
    qmlRegisterType<InputDeviceEvent>( uri, 1, 0, "PhxInputDeviceEvent" );
    qmlRegisterType<QMLInputDevice>( uri, 1, 0, "PhxQMLInputDevice" );

    qRegisterMetaType<InputDevice *>( "InputDevice *" );
}
