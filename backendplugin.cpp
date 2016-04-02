#include "backendplugin.h"

#include "corecontrolproxy.h"
#include "corecontrol.h"
#include "core.h"
#include "control.h"
#include "libretrocore.h"
#include "videooutput.h"

#include "inputmanager.h"
#include "qmlinputdevice.h"

#include <qqml.h>

void CorePlugin::registerTypes( const char *uri ) {
    qmlRegisterType<VideoOutput>( uri, 1, 0, "VideoOutput" );
    qmlRegisterType<CoreControlProxy>( uri, 1, 0, "CoreControl" );
    qmlRegisterUncreatableType<ControlHelper>( uri, 1, 0, "Control", "Control or its subclasses cannot be instantiated from QML." );

    qRegisterMetaType<Control::State>( "Control::State" );
    qRegisterMetaType<ControlHelper::State>( "ControlHelper::State" );
    qRegisterMetaType<size_t>( "size_t" );
    qRegisterMetaType<QStringMap>();
    qRegisterMetaType<ProducerFormat>( "ProducerFormat");
    qRegisterMetaType<VideoOutput *>( "VideoOutput *");

    qmlRegisterType<InputManager>( uri, 1, 0, "InputManager" );
    qmlRegisterType<InputDeviceEvent>( uri, 1, 0, "InputDeviceEvent" );
    qmlRegisterType<QMLInputDevice>( uri, 1, 0, "QMLInputDevice" );

    qRegisterMetaType<InputDevice *>( "InputDevice *" );
}



