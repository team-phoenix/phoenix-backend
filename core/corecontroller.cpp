#include "corecontroller.h"

CoreController::CoreController( QObject *parent ) : QObject( parent ),
    muted( false ), playbackRate( 1.0 ), source(), state( QStringLiteral( "init" ) ), volume( 1.0 ),
    coreThread( new QThread() ), coreModel( new CoreModel() ) {

    // Set up the thread and move coreModel to it
    coreThread->setObjectName( "Core thread" );
    coreModel->moveToThread( coreThread );

    // Catch the user exit signal and clean up
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [ = ]() {

        qCDebug( phxController ) << "===========QCoreApplication::aboutToQuit()===========";

        // Tell the model to stop gracefully

        // Stop the core thread
        coreThread->exit();
        coreThread->wait();
        coreThread->deleteLater();

    } );

    // Make sure that the model gets deleted once its thread finishes processing all events
    connect( coreThread, &QThread::finished, coreModel, &CoreModel::deleteLater );

    // Launch the thread
    coreThread->start();
}

void CoreController::load() {
    // check if in "stopped" state, error if not

    // check source for "type" key, error if not found

    // switch( "type" value )
        // case "libretro":
            // check for "core", "game", error if not found
            // check the two exist and are readable by Phoenix, error if not able
        // default
            // error, invalid value for "type"

    // enter loading state
}

void CoreController::play() {
    // check if in "paused" state, error if not

}

void CoreController::pause() {
    // check if in "playing" state, error if not

}

void CoreController::stop() {
    // check if in "playing", "paused" or "error" state, error if not

    // if "playing" or "paused", tell model to end session gracefully
    // otherwise, tell model to end session not so gracefully

}

void CoreController::reset() {
    // check if in "playing" or "paused" state, error if not

    // check if we're able to reset or not, error if we can't

    // send signal to model to reset emulation
}

QString CoreController::getState() {
    return state;
}

