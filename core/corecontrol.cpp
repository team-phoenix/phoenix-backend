#include "corecontrol.h"

// Public

CoreControl::CoreControl( QObject *parent ) : QObject( parent ),
    looper( nullptr ),
    looperThread( new QThread() ),
    core( nullptr ),
    coreThread( new QThread() ),
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

void CoreControl::loadLibretro() {

    // Create LibretroCore
    LibretroCore *libretroCore = new LibretroCore();
    core = libretroCore;
    libretroCore->moveToThread( coreThread );
    coreThread->setObjectName( QStringLiteral( "Core thread (libretro)" ) );

    // Create Looper
    looper = new Looper();
    looper->moveToThread( looperThread );
    looperThread->setObjectName( QStringLiteral( "Looper thread" ) );

    // Connect Looper
    connect( this, &CoreControl::beginLooper, looper, &Looper::beginLoop );
    emit beginLooper( ( 1.0 / 60.0 ) * 1000.0 );

    // Connect InputManager
    connect( looper, &Looper::signalFrame, inputManager, &InputManager::pollStates );
    connect( dynamic_cast<QObject *>( inputManager ), SIGNAL( producerFormat( ProducerFormat ) ),
             dynamic_cast<QObject *>( libretroCore ), SLOT( consumerFormat( ProducerFormat ) ) );
    connect( dynamic_cast<QObject *>( inputManager ), SIGNAL( producerData( QString, QMutex *, void *, size_t ) ),
             dynamic_cast<QObject *>( libretroCore ), SLOT( consumerData( QString, QMutex *, void *, size_t ) ) );

    // Connect LibretroCore to the proxy system
    connectCoreProxy();

    // For debugging
    connect( libretroCore, &Core::stateChanged, [ = ]( Core::State newState ) {
        qCDebug( phxController ) << Q_FUNC_INFO << "State change to"
                                 << newState << "acknowledged";
    } );

    // Connect the consumers
    audioOutput = new AudioOutput();
    audioOutput->moveToThread( audioOutputThread );
    audioOutputThread->setObjectName( "Audio thread (libretro)" );

    // The old syntax is necessary here as we're doing some fancy tricks to work around some limitations of Qt.
    // Qt does not support QObjects inheriting from multple QObjects, but we want Producer and Consumer to provide common
    // signals and slots. So, we declare the signals and slots anyway and just have Producer and Consumer be plain classes.
    // Thanks to peppe and thiago from #Qt on Freenode for the idea
    connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( producerFormat( ProducerFormat ) ),
             dynamic_cast<QObject *>( audioOutput ), SLOT( consumerFormat( ProducerFormat ) ) );
    connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( producerData( QString, QMutex *, void *, size_t ) ),
             dynamic_cast<QObject *>( audioOutput ), SLOT( consumerData( QString, QMutex *, void *, size_t ) ) );

    connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( producerFormat( ProducerFormat ) ),
             dynamic_cast<QObject *>( videoOutput ), SLOT( consumerFormat( ProducerFormat ) ) );
    connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( producerData( QString, QMutex *, void *, size_t ) ),
             dynamic_cast<QObject *>( videoOutput ),  SLOT( consumerData( QString, QMutex *, void *, size_t ) ) );

    // Start threads
    looperThread->start();
    coreThread->start();
    audioOutputThread->start();

}
