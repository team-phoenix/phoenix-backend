#include "coreplugin.h"

#include "corecontrolproxy.h"
#include "corecontrol.h"
#include "core.h"
#include "control.h"
#include "libretrocore.h"

void CorePlugin::registerTypes( const char *uri ) {
    qmlRegisterType<CoreControlProxy>( uri, 1, 0, "PhxCoreControl" );
    qmlRegisterUncreatableType<ControlHelper>( uri, 1, 0, "Control", "Control or its subclasses cannot be instantiated from QML." );

    qRegisterMetaType<Control::State>( "Control::State" );
    qRegisterMetaType<ControlHelper::State>( "ControlHelper::State" );
    qRegisterMetaType<size_t>( "size_t" );
    qRegisterMetaType<QStringMap>();
    qRegisterMetaType<ProducerFormat>();
}
