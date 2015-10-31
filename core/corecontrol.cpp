#include "corecontrol.h"

CoreControl::CoreControl( QObject *parent ) : QObject( parent ),
    core( nullptr ),
    coreThread( new QThread() ) {

    // Notify the QML engine of all the initial property values set in this constructor
    // notifyAllProperties();

    // Set up the thread and move gameSession to it
    coreThread->setObjectName( "Game thread" );
    core->moveToThread( coreThread );

    // Catch the program exit signal and clean up
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [ = ]() {

        qCDebug( phxController ) << "===========QCoreApplication::aboutToQuit()===========";

        // Tell gameSession to shut down
        // emit signalShutdown();

        // Stop the game thread
        // UI will be blocked until shutdown command is processed
        coreThread->exit();
        coreThread->wait();
        coreThread->deleteLater();

    } );

    // Make sure that the session gets deleted once its thread finishes processing all events
    connect( coreThread, &QThread::finished, core, &Core::deleteLater );

    // Launch the thread
    coreThread->start();

    // setState( Core::STOPPED );
}
