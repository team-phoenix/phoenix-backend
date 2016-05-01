#include "backendplugin.h"

#include <QDebug>

void BackendPlugin::registerTypes( const char *uri ) {
    // Register our custom types for use within QML
    qmlRegisterType<ControlOutput>( uri, 1, 0, "ControlOutput" );
    qmlRegisterType<GlobalGamepad>( uri, 1, 0, "GlobalGamepad" );
    qmlRegisterType<PhoenixWindow>( uri, 1, 0, "PhoenixWindow" );
    qmlRegisterType<PhoenixWindowNode>( uri, 1, 0, "PhoenixWindowNode" );
    qmlRegisterType<VideoOutput>( uri, 1, 0, "VideoOutput" );
    qmlRegisterType<VideoOutputNode>( uri, 1, 0, "VideoOutputNode" );
    qmlRegisterType<GameConsole>( uri, 1, 0, "GameConsole" );
    qmlRegisterUncreatableType<ControlHelper>( uri, 1, 0, "Control", "Control or its subclasses cannot be instantiated from QML." );

    // Needed for connecting signals/slots
    qRegisterMetaType<Node::Command>( "Command" );
    qRegisterMetaType<Node::DataType>( "DataType" );
    qRegisterMetaType<Node::State>( "State" );
    qRegisterMetaType<QStringMap>();
    qRegisterMetaType<size_t>( "size_t" );
    qRegisterMetaType<ProducerFormat>();
    qmlRegisterUncreatableType<Node>( uri, 1, 0, "Node", "Node or its subclasses cannot be instantiated from QML." );

}
