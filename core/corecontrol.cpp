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
    //     qCDebug( phxControl ) << "===========QCoreApplication::aboutToQuit()===========";
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
     * TODO: Explanation
     */

    // LibretroCore (Producer and Consumer)

    // Create LibretroCore
    LibretroCore *libretroCore = new LibretroCore();
    core = libretroCore;
    libretroCore->moveToThread( coreThread );
    coreThread->setObjectName( QStringLiteral( "Core thread (libretro)" ) );

    // Connect LibretroCore to the proxy system
    connectCoreProxy();

    // Connect LibretroCore to this so we can grab the current session's native framerate
    connect( libretroCore, &LibretroCore::libretroCoreNativeFramerate, this, &CoreControl::libretroCoreNativeFramerate );

    // Forward LibretroCore control signals as our own
    connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( stateChanged( Control::State ) ),
             dynamic_cast<QObject *>( this ), SIGNAL( setState( Control::State ) ) );

    // Looper (Control/Controllable)
    // TODO: Make sure to check if in some different timer mode and use a different timing source

    // Create Looper
    looper = new Looper();
    // looper->moveToThread( looperThread );
    // looperThread->setObjectName( QStringLiteral( "Looper thread (libretro)" ) );

    // Connect this (controller) to Looper
    CONNECT_CONTROL_CONTROLLABLE( this, looper );

    // InputManager (Producer)

    // inputManager is a property and a reference to it should be set by this point
    // Connect InputManager to LibretroCore (produces input data which also drives frame production in LibretroCore)
    CONNECT_PRODUCER_CONSUMER( inputManager, libretroCore );

    // Connect control signals to InputManager
    CONNECT_CONTROL_CONTROLLABLE( this, inputManager );

    // Connect Looper to InputManager (drive input polling)
    connect( looper, &Looper::timeout, inputManager, &InputManager::pollStates );

    // Consumers

    // Create the consumers
    audioOutput = new AudioOutput();
    audioOutput->moveToThread( audioOutputThread );
    audioOutputThread->setObjectName( "Audio thread (libretro)" );

    // VideoOutput (Consumer) is a property and a reference to it should be set by this point

    // Connect LibretroCore to the consumers (AV output)
    CONNECT_PRODUCER_CONSUMER( libretroCore, audioOutput );
    CONNECT_PRODUCER_CONSUMER( libretroCore, videoOutput );

    // Connect control signals to consumers
    CONNECT_CONTROL_CONTROLLABLE( this, audioOutput );
    CONNECT_CONTROL_CONTROLLABLE( this, videoOutput );

    // Start threads
    looperThread->start( QThread::TimeCriticalPriority );
    coreThread->start( QThread::HighPriority );
    audioOutputThread->start();

}

void CoreControl::libretroCoreNativeFramerate( qreal framerate ) {
    // Framerate will only ever be set during loading
    // TODO: Make sure to check if in some different timer mode
    Q_ASSERT( state == Control::LOADING );
    libretroCoreFPS = framerate;
    // qCDebug( phxControl ).nospace() << "Native framerate: " << libretroCoreFPS << "fps, setting InputManager and restarting Looper...";
    emit stopLooper();
    emit setFramerate( libretroCoreFPS );
    emit startLooper( ( 1.0 / libretroCoreFPS ) * 1000.0 );
}

void CoreControl::coreStateChanged( Control::State state ) {
    emit setState( state );
}
