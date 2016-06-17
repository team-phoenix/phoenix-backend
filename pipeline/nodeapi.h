#pragma once

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QThread>

#include "node.h"
#include "pipelinecommon.h"

// Nodes
#include "audiooutput.h"
#include "sdlmanager.h"
#include "controloutput.h"
#include "gameconsole.h"
#include "globalgamepad.h"
#include "libretroloader.h"
#include "libretrorunner.h"
#include "microtimer.h"
#include "phoenixwindow.h"
#include "phoenixwindownode.h"
#include "remapper.h"
#include "remappermodel.h"
#include "sdlunloader.h"
#include "videooutput.h"
#include "videooutputnode.h"

class NodeAPI : public QObject {
        Q_OBJECT

    public:
        NodeAPI();

        enum class Thread {
            Main = 0,
            Game,
        };
        Q_ENUM( Thread )

        enum class Pipeline {
            Default = 0,
            Libretro,
        };
        Q_ENUM( Pipeline )

        // Register your Node subclass so it can be used with pipelines
        static void registerNode( Node *node, Thread nodeThread = Thread::Game, QStringList nodeDependencies = {} );

        // Register your non-Node QObject so it can be used with Nodes
        static void registerNonNode( QObject *object, Thread objThread = Thread::Main );

    private: // Pipelines
        static QStringMap defaultPipeline;
        static bool checkDefaultPipeline();
        static void connectDefaultPipeline();
        static void disconnectDefaultPipeline();

        static QStringMap libretroPipeline;
        static bool checkLibretroPipeline();
        static void connectLibretroPipeline();
        static void disconnectLibretroPipeline();

    private: // Node API internals
        static Pipeline currentPipeline;
        static bool currentlyAssembling;
        static QMap<QString, QStringList> nonNodeDependencies;
        static QMap<QString, Node *> nodes;
        static QMap<QString, QObject *> nonNodes;
        static QThread *gameThread;

        // Used with Nodes and non-Nodes alike
        static QMap<QString, Thread> threads;

        // Check if the current pipeline is ready, assemble if it is
        static void nodeCheck();
};
