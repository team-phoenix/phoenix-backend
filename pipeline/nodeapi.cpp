#include "nodeapi.h"

NodeAPI::NodeAPI() {
    gameThread = new QThread();
    gameThread->setObjectName( "Game thread" );
    gameThread->start();
}

// Public
void NodeAPI::registerNode( Node *node, NodeAPI::Thread nodeThread, QStringList nodeDependencies ) {
    QString className = node->metaObject()->className();
    nodes[ className ] = node;
    threads[ className ] = nodeThread;
    nonNodeDependencies[ className ] = nodeDependencies;
    qDebug() << node << nodeThread << nodeDependencies;

    pipelineHeartbeat();
}

void NodeAPI::registerNonNode( QObject *object, NodeAPI::Thread objThread ) {
    QString className = object->metaObject()->className();
    nonNodes[ className ] = object;
    threads[ className ] = objThread;
    qDebug() << object << objThread;

    pipelineHeartbeat();
}

QMap<QString, QStringList> NodeAPI::defaultPipeline = {
    { QT_STRINGIFY( GameConsole ), { QT_STRINGIFY( PhoenixWindowNode ) } },
    { QT_STRINGIFY( PhoenixWindowNode ), { QT_STRINGIFY( MicroTimer ) } },
    { QT_STRINGIFY( MicroTimer ), { QT_STRINGIFY( SDLManager ) } },
    { QT_STRINGIFY( SDLManager ), { QT_STRINGIFY( Remapper ) } },
    { QT_STRINGIFY( Remapper ), { QT_STRINGIFY( GlobalGamepad ), QT_STRINGIFY( ControlOutput ), QT_STRINGIFY( SDLUnloader ) } },
};

QMap<QString, QStringList> NodeAPI::libretroPipeline = {
    { QT_STRINGIFY( GameConsole ), { QT_STRINGIFY( PhoenixWindowNode ) } },
    { QT_STRINGIFY( PhoenixWindowNode ), { QT_STRINGIFY( LibretroLoader ) } },
    { QT_STRINGIFY( LibretroLoader ), { QT_STRINGIFY( MicroTimer ) } },
    { QT_STRINGIFY( MicroTimer ), { QT_STRINGIFY( SDLManager ) } },
    { QT_STRINGIFY( SDLManager ), { QT_STRINGIFY( Remapper ) } },
    { QT_STRINGIFY( Remapper ), { QT_STRINGIFY( GlobalGamepad ), QT_STRINGIFY( LibretroVariableForwarder ) } },
    { QT_STRINGIFY( LibretroVariableForwarder ), { QT_STRINGIFY( LibretroRunner ) } },
    {
        QT_STRINGIFY( LibretroRunner ), {
            QT_STRINGIFY( SDLUnloader ), QT_STRINGIFY( AudioOutput ), QT_STRINGIFY( ControlOutput ), QT_STRINGIFY( VideoOutputNode )
        }
    },
};

bool NodeAPI::requiredNodesAvailable( QMap<QString, QStringList> pipeline ) {
    QSet<QString> neededNodes;
    QSet<QString> availableNodes = nodes.keys().toSet();

    for( QString parent : pipeline.keys() ) {
        for( QString child : pipeline[ parent ] ) {
            neededNodes.insert( child );
        }
    }

    // If we're ready neededNodes will be empty
    neededNodes -= availableNodes;

    QSet<QString> neededNonNodes;
    QSet<QString> availableNonNodes = nonNodes.keys().toSet();

    for( QString key : nonNodeDependencies.keys() ) {
        for( QString value : nonNodeDependencies[ key ] ) {
            neededNonNodes.insert( value );
        }
    }

    neededNonNodes -= availableNonNodes;

    return neededNodes.isEmpty() && neededNonNodes.isEmpty();
}

void NodeAPI::connectPipeline( QMap<QString, QStringList> pipeline ) {
    for( QString parentString : pipeline.keys() ) {
        for( QString childString : pipeline[ parentString ] ) {
            qDebug().noquote() << "Connect" << parentString << "->" << childString;
            Node *parent = nodes[ parentString ];
            Node *child = nodes[ childString ];
            connectNodes( parent, child );
        }
    }

    // Move all Nodes to their threads
    for( QString nodeString : nodes.keys() ) {
        if( threads[ nodeString ] == Thread::Game ) {
            qDebug().noquote() << "Moving" << nodeString << "to game thread";

            if( nodes.contains( nodeString ) ) {
                nodes[ nodeString ]->moveToThread( gameThread );
            } else {
                nonNodes[ nodeString ]->moveToThread( gameThread );
            }
        }
    }

    // Have the Nodes connect their dependencies, if they have any
    // FIXME: Threading? Queued connections?
    for( Node *node : nodes ) {
        node->connectDependencies( nonNodes );
    }
}

void NodeAPI::disconnectPipeline( QMap<QString, QStringList> pipeline ) {
    for( QString parentString : pipeline.keys() ) {
        for( QString childString : pipeline[ parentString ] ) {
            qDebug().noquote() << "Connect" << parentString << "->" << childString;
            Node *parent = nodes[ parentString ];
            Node *child = nodes[ childString ];
            disconnectNodes( parent, child );
        }
    }

    // Have the Nodes connect their dependencies, if they have any
    // FIXME: Threading? Queued connections?
    for( Node *node : nodes ) {
        node->disconnectDependencies( nonNodes );
    }
}

// Private (Node API internals)

bool NodeAPI::currentlyAssembling = true;
QMap<QString, QStringList> NodeAPI::nonNodeDependencies;
QMap<QString, Node *> NodeAPI::nodes;
QMap<QString, QObject *> NodeAPI::nonNodes;
QMap<QString, NodeAPI::Thread> NodeAPI::threads;
QThread *NodeAPI::gameThread;

// Initial state is to immediately assemble the default pipeline
NodeAPI::Pipeline NodeAPI::currentPipeline = NodeAPI::Pipeline::Default;
NodeAPI::State NodeAPI::state = NodeAPI::State::Assembling;

void NodeAPI::pipelineHeartbeat() {
    qDebug() << state;

    switch( state ) {
        case State::Inactive: {
            // TODO: Read from somewhere what pipeline to assemble next?
            break;
        }

        case State::Assembling: {
            switch( currentPipeline ) {
                case Pipeline::Default: {
                    if( requiredNodesAvailable( defaultPipeline ) ) {
                        connectPipeline( defaultPipeline );
                        state = State::Active;
                        qDebug() << state;
                    }

                    break;
                }

                case Pipeline::Libretro: {
                    break;
                }

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }
}
