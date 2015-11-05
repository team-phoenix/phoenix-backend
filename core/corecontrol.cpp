#include "corecontrol.h"

// Public

CoreControl::CoreControl( QObject *parent ) : QObject( parent ),
    looper( nullptr ),
    looperThread( new QThread() ),
    core( nullptr ),
    coreThread( new QThread() ),
    libretroCoreFPS( 60.0 ),
    audioOutputThread( new QThread() ),
    pausable( false ),
    playbackSpeed( 1.0 ),
    resettable( false ),
    rewindable( false ),
    source(),
    state( Core::INIT ),
    volume( 1.0 ) {

    // Notify the QML engine of all the initial property values set in this constructor
    notifyAllProperties();

    // Set up the thread and move gameSession to it
    // coreThread->setObjectName( "Game thread" );
    // core->moveToThread( coreThread );
    //
    // // Catch the program exit signal and clean up
    // connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [ = ]() {
    //
    //     qCDebug( phxController ) << "===========QCoreApplication::aboutToQuit()===========";
    //
    //     // Tell gameSession to shut down
    //     // emit signalShutdown();
    //
    //     // Stop the game thread
    //     // UI will be blocked until shutdown command is processed
    //     coreThread->exit();
    //     coreThread->wait();
    //     coreThread->deleteLater();
    //
    // } );
    //
    // // Make sure that the session gets deleted once its thread finishes processing all events
    // connect( coreThread, &QThread::finished, core, &Core::deleteLater );
    //
    // // Launch the thread
    // coreThread->start();

}

CoreControl::~CoreControl() {
}

// Private

void CoreControl::initLibretro() {
    /*
     * Signals trickle down from Looper to InputManager to LibretroCore to the consumers.
     * In order to set Looper to the proper framerate (coreFPS), we hook the state change to PLAYING as that's the
     * earliest we can be sure it was set and manually do the first frame ourselves. Once we've done this CoreController
     * will get coreFPS from LibretroCore and we can turn around and pass it from the top down, properly.
     */

    // Create LibretroCore (Producer and Consumer)
    LibretroCore *libretroCore = new LibretroCore();
    core = libretroCore;
    libretroCore->moveToThread( coreThread );
    coreThread->setObjectName( QStringLiteral( "Core thread (libretro)" ) );

    // Connect LibretroCore to the proxy system
    connectCoreProxy();

    // Connect LibretroCore to this so we can grab the current session's native framerate
    connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( producerFormat( ProducerFormat ) ),
             this, SLOT( consumerFormat( ProducerFormat ) ) );

    // Looper (control)

    // Create Looper
    looper = new Looper();
    looper->moveToThread( looperThread );
    looperThread->setObjectName( QStringLiteral( "Looper thread (libretro)" ) );

    // Connect this (controller) to Looper
    connect( this, &CoreControl::startLooper, looper, &Looper::beginLoop );
    connect( this, &CoreControl::stopLooper, looper, &Looper::endLoop );

    // InputManager (Producer)

    // inputManager is a property and a reference to it should be set by this point
    // Connect InputManager to LibretroCore (produces input data which also drives frame production in LibretroCore)
    CONNECT_PRODUCER_CONSUMER( inputManager, libretroCore );

    // Connect this to InputManager (set framerate? FIXME: why?)
    connect( this, &CoreControl::setFramerate, inputManager, &InputManager::setPollRate );

    // Connect Looper to InputManager (drive input polling)
    connect( looper, &Looper::signalFrame, inputManager, &InputManager::pollStates );

    // Consumers

    // Create the consumers
    audioOutput = new AudioOutput();
    audioOutput->moveToThread( audioOutputThread );
    audioOutputThread->setObjectName( "Audio thread (libretro)" );

    // VideoOutput (Consumer) is a property and a reference to it should be set by this point

    // Connect LibretroCore to the consumers (AV output)
    CONNECT_PRODUCER_CONSUMER( libretroCore, audioOutput );
    CONNECT_PRODUCER_CONSUMER( libretroCore, videoOutput );

    // Start threads
    looperThread->start( QThread::TimeCriticalPriority );
    coreThread->start( QThread::HighPriority );
    audioOutputThread->start();

}

void CoreControl::consumerFormat( ProducerFormat consumerFmt ) {
    // Framerate will only ever be set during loading
    if( state == Core::LOADING ) {
        libretroCoreFPS = consumerFmt.videoFramerate;
        // qCDebug( phxController ).nospace() << "Native framerate: " << libretroCoreFPS << "fps, setting InputManager and restarting Looper...";
        emit stopLooper();
        emit setFramerate( libretroCoreFPS );
        emit startLooper( ( 1.0 / libretroCoreFPS ) * 1000.0 );
    }
}
