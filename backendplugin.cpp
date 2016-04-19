#include "backendplugin.h"

#include "gameconsoleproxy.h"
#include "gameconsole.h"
#include "core.h"
#include "control.h"
#include "libretrocore.h"
#include "videooutput.h"

#include "keyboard.h"

#include "gamepadmanager.h"
#include "inputdevice.h"

#include <QtQml>

using namespace Input;

void CorePlugin::registerTypes( const char *uri ) {
    qmlRegisterType<VideoOutput>( uri, 1, 0, "VideoOutput" );
    qmlRegisterType<GameConsoleProxy>( uri, 1, 0, "GameConsole" );
    qmlRegisterUncreatableType<ControlHelper>( uri, 1, 0, "Control", "Control or its subclasses cannot be instantiated from QML." );

    qRegisterMetaType<Control::State>( "Control::State" );
    qRegisterMetaType<ControlHelper::State>( "ControlHelper::State" );
    qRegisterMetaType<size_t>( "size_t" );
    qRegisterMetaType<LibretroCore::QStringMap>( "QStringMap" );
    qRegisterMetaType<ProducerFormat>( "ProducerFormat");
    qRegisterMetaType<VideoOutput *>( "VideoOutput *");


}



