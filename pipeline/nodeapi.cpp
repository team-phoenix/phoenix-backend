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

    nodeCheck();
}

void NodeAPI::registerNonNode( QObject *object, NodeAPI::Thread objThread ) {
    QString className = object->metaObject()->className();
    nonNodes[ className ] = object;
    threads[ className ] = objThread;
    qDebug() << object << objThread;

    nodeCheck();
}

// FIXME: Value should be a list
QStringMap NodeAPI::defaultPipeline = {
    { QT_STRINGIFY( GameConsole ), QT_STRINGIFY( PhoenixWindowNode ) },
    { QT_STRINGIFY( PhoenixWindowNode ), QT_STRINGIFY( MicroTimer ) },
    { QT_STRINGIFY( MicroTimer ), QT_STRINGIFY( SDLManager ) },
    { QT_STRINGIFY( SDLManager ), QT_STRINGIFY( Remapper ) },
    { QT_STRINGIFY( Remapper ), QT_STRINGIFY( GlobalGamepad ) },
    { QT_STRINGIFY( Remapper ), QT_STRINGIFY( ControlOutput ) },
    { QT_STRINGIFY( Remapper ), QT_STRINGIFY( SDLUnloader ) },
};

bool NodeAPI::checkDefaultPipeline() {
    QSet<QString> neededNodes = defaultPipeline.values().toSet();
    QSet<QString> availableNodes = nodes.keys().toSet();

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

void NodeAPI::connectDefaultPipeline() {
    for( QString parentString : defaultPipeline.keys() ) {
        QString childString = defaultPipeline[ parentString ];
        qDebug().noquote() << "Connect" << parentString << "->" << childString;
        Node *parent = nodes[ parentString ];
        Node *child = nodes[ childString ];
        connectNodes( parent, child );
    }

    // Move all Nodes to their threads
    for( QString nodeString : nodes.keys() ) {
        if( threads[ nodeString ] == Thread::Game ) {
            qDebug().noquote() << "Moving" << nodeString << "to game thread";
            nodes[ nodeString ]->moveToThread( gameThread );
        }
    }

    // Have the Nodes connect their dependencies, if they have any
    for( Node *node : nodes ) {
        node->connectDependencies( nonNodes );
    }
}

void NodeAPI::disconnectDefaultPipeline() {
}

QStringMap NodeAPI::libretroPipeline = {
    { QT_STRINGIFY( GameConsole ), QT_STRINGIFY( PhoenixWindowNode ) },
    { QT_STRINGIFY( PhoenixWindowNode ), QT_STRINGIFY( LibretroLoader ) },
    { QT_STRINGIFY( LibretroLoader ), QT_STRINGIFY( MicroTimer ) },
    { QT_STRINGIFY( MicroTimer ), QT_STRINGIFY( SDLManager ) },
    { QT_STRINGIFY( SDLManager ), QT_STRINGIFY( Remapper ) },
    { QT_STRINGIFY( Remapper ), QT_STRINGIFY( GlobalGamepad ) },
    { QT_STRINGIFY( Remapper ), QT_STRINGIFY( LibretroVariableForwarder ) },
    { QT_STRINGIFY( LibretroVariableForwarder ), QT_STRINGIFY( LibretroRunner ) },
    { QT_STRINGIFY( LibretroRunner ), QT_STRINGIFY( SDLUnloader ) },
    { QT_STRINGIFY( LibretroRunner ), QT_STRINGIFY( AudioOutput ) },
    { QT_STRINGIFY( LibretroRunner ), QT_STRINGIFY( ControlOutput ) },
    { QT_STRINGIFY( LibretroRunner ), QT_STRINGIFY( VideoOutputNode ) },
};

bool NodeAPI::checkLibretroPipeline() {
    QSet<QString> neededNodes = libretroPipeline.values().toSet();
    QSet<QString> availableNodes = nodes.keys().toSet();

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

void NodeAPI::connectLibretroPipeline() {

}

void NodeAPI::disconnectLibretroPipeline() {

}

// Private (Node API internals)

NodeAPI::Pipeline NodeAPI::currentPipeline = NodeAPI::Pipeline::Default;
bool NodeAPI::currentlyAssembling = true;
QMap<QString, QStringList> NodeAPI::nonNodeDependencies;
QMap<QString, Node *> NodeAPI::nodes;
QMap<QString, QObject *> NodeAPI::nonNodes;
QMap<QString, NodeAPI::Thread> NodeAPI::threads;
QThread *NodeAPI::gameThread;

void NodeAPI::nodeCheck() {
    if( currentlyAssembling ) {
        switch( currentPipeline ) {
            case Pipeline::Default: {
                if( checkDefaultPipeline() ) {
                    connectDefaultPipeline();
                    currentlyAssembling = false;
                }

                break;
            }

            case Pipeline::Libretro: {
                break;
            }

            default:
                break;
        }
    } else {
        qDebug() << "nodeCheck() called, not currently assembling a pipeline";
    }
}
