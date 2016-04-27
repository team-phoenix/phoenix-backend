#include "backendplugin.h"

#include <pipelinenode.h>

#include "gameconsoleproxy.h"
#include "gameconsole.h"
#include "core.h"
#include "libretrocore.h"
#include "videooutput.h"

#include "keyboard.h"

#include "remapmodel.h"
#include "gamepadmodel.h"
#include "globalgamepad.h"
#include "gamepadmanager.h"
#include "inputdevice.h"

#include <QtQml>

using namespace Input;

void CorePlugin::registerTypes( const char *uri ) {
    qmlRegisterType<VideoOutput>( uri, 1, 0, "VideoOutput" );
    qmlRegisterType<GameConsoleProxy>( uri, 1, 0, "GameConsole" );
    qmlRegisterSingletonType<GamepadModel>( uri, 1, 0, "GamepadModel", GamepadModel::registerSingletonCallback );
    qmlRegisterSingletonType<RemapModel>( uri, 1, 0, "RemapModel", RemapModel::registerSingletonCallback );
    qmlRegisterType<GlobalGamepad>( uri, 1, 0, "GlobalGamepad" );
    qmlRegisterUncreatableType<ControlHelper>( uri, 1, 0, "Control", "Control or its subclasses cannot be instantiated from QML." );

    qRegisterMetaType<GameConsoleProxy::PlaybackState>( "GameConsole::PlaybackState" );
    qRegisterMetaType<ControlHelper::State>( "ControlHelper::State" );
    qRegisterMetaType<size_t>( "size_t" );
    qRegisterMetaType<AVFormat>();
    qRegisterMetaType<VideoOutput *>( "VideoOutput *" );
    qRegisterMetaType<Gamepad *>( "Gamepad *" );

    qRegisterMetaType<Command>( "Command" );
    qRegisterMetaType<PipelineState>( "PipelineState" );
    qRegisterMetaType<DataType>( "DataType" );
}



