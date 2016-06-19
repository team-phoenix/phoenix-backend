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

class Pipeline : public QObject {
        Q_OBJECT

    public:
        Pipeline();

        enum class Thread {
            Main = 0,
            Game,
        };
        Q_ENUM( Thread )

        enum PipelineType {
            Default = 0,
            Libretro,
        };
        Q_ENUM( PipelineType )

        enum class State {
            Inactive = 0,
            Assembling,
            Active,
            Disassembling
        };
        Q_ENUM( State )

        // Register your Node subclass so it can be used with pipelines
        static void registerNode( Node *node, Thread nodeThread = Thread::Game, QStringList nodeDependencies = {} );

        // Register your non-Node QObject so it can be used with Nodes
        static void registerNonNode( QObject *object, Thread objThread = Thread::Main );

        // Disconnect current pipeline and begin assembling the specified type
        // TODO: Notify caller when complete somehow
        static void changePipeline( PipelineType type );

        static State getCurrentState();
        static PipelineType getCurrentPipeline();

    private: // Pipelines
        static QMap<QString, QStringList> defaultPipeline;
        static QMap<QString, QStringList> libretroPipeline;
        static bool requiredNodesAvailable( QMap<QString, QStringList> pipeline );
        static void connectPipeline( QMap<QString, QStringList> pipeline );
        static void disconnectPipeline( QMap<QString, QStringList> pipeline );

    private: // Node API internals
        static QMap<QString, QStringList> nonNodeDependencies;
        static QMap<QString, Node *> nodes;
        static QMap<QString, QObject *> nonNodes;
        static QThread *gameThread;

        static PipelineType currentPipeline;
        static State state;

        // Used with Nodes and non-Nodes alike
        static QMap<QString, Thread> threads;

        // Preform actions based on the current state
        static void pipelineHeartbeat();
};
