#include "corecontrol.h"

// Public

CoreControl::CoreControl( QObject *parent ) : QObject( parent ),
    threadChildren(),
    gameThreadChildren(),
    looper( nullptr ),
    inputManager( nullptr ),
    core( nullptr ),
    audioOutput( nullptr ),
    audioOutputThread( nullptr ),
    videoOutput( nullptr ),
    vsync( false ),
    connectionList() {

}

CoreControl::~CoreControl() {
}

// Public slots

void CoreControl::shutdown() {
    if( state != Control::STOPPED && core ) {
        qCDebug( phxControl ) << "Stopping running game...";
        stop();
    } else if( !core ) {
        qCDebug( phxControl ) << "No core running, continuing";
        cleanup();
    } else {
        qCDebug( phxControl ) << "Core has already stopped, continuing";
        cleanup();
    }

}

void CoreControl::setVideoOutput( VideoOutput *videoOutput ) {
    // This is relevant to us only, so we'll immediately acknowledge
    this->videoOutput = videoOutput;
    emit videoOutputChanged( videoOutput );
}

void CoreControl::setInputManager( InputManager *inputManager ) {
    // This is relevant to us only, so we'll immediately acknowledge
    this->inputManager = inputManager;
    emit inputManagerChanged( inputManager );
}

void CoreControl::setPlaybackSpeed( qreal playbackSpeed ) {
    emit setPlaybackSpeedForwarder( playbackSpeed );
}

void CoreControl::setSource( QStringMap source ) {
    // Determine Core type, instantiate appropiate type
    // We don't store it locally, we just tap into it
    if( source[ QStringLiteral( "type" ) ] == QStringLiteral( "libretro" ) ) {
        initLibretroCore();
        // qCDebug( phxControl ) << "LibretroCore fully initalized and connected";
    } else {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
                                           << source[ "type" ] << QStringLiteral( " passed to load()!" );
        return;
    }

    // Send it to Core
    emit setSourceForwarder( source );
}

void CoreControl::setVolume( qreal volume ) {
    emit setVolumeForwarder( volume );
}

void CoreControl::setVsync( bool vsync ) {
    // This is relevant to us only, so we'll immediately acknowledge
    this->vsync = vsync;
    emit vsyncChanged( vsync );
}

void CoreControl::load() {
    emit loadForwarder();
}

void CoreControl::play() {
    emit playForwarder();
}

void CoreControl::pause() {
    emit pauseForwarder();
}

void CoreControl::stop() {
    emit stopForwarder();
}

void CoreControl::reset() {
    emit resetForwarder();
}

// Private

void CoreControl::deleteThreads() {

    if( threadChildren.size() ) {
        qCDebug( phxControl ) << "Shutting down threads";

        for( QThread *thread_ : threadChildren.keys() ) {
            qCDebug( phxControl ).nospace() << "Shutting down thread: " << thread_ << "...";

            // No harm in connecting right before the thread dies!
            for( QObject *obj : threadChildren[ thread_ ] ) {
                qCDebug( phxControl ) << "    Shutting down:" << obj;
                Q_ASSERT( obj->thread() != thread() );
                connect( thread_, &QThread::finished, obj, &QObject::deleteLater );
            }

            // Terminate the thread
            thread_->quit();
            thread_->wait();
            thread_->deleteLater();
        }

        threadChildren.clear();
    } else {
        qCDebug( phxControl ) << "No threads to delete, continuing";
    }
}

void CoreControl::deleteGameThreadChildren() {
    if( gameThreadChildren.size() ) {
        qCDebug( phxControl ) << "Shutting down game thread children";

        for( QObject *obj : gameThreadChildren ) {
            qCDebug( phxControl ) << "    Shutting down:" << obj;
            Q_ASSERT( obj->thread() == thread() );
            delete obj;
        }

        gameThreadChildren.clear();
    } else {
        qCDebug( phxControl ) << "No children in game thread to delete, skipping";
    }
}

void CoreControl::cleanup() {
    disconnectConnections();
    deleteThreads();
    deleteGameThreadChildren();
    qCDebug( phxControl ) << "Fully unloaded";
}

void CoreControl::connectCoreForwarder() {
    // Forward these signals that are important to QML (but not us) to whoever's concerned (such as Core)
    // In some rare cases we DO hook our own signal to do stuff, but not generally
    connect( core, &Core::pausableChanged, this, &CoreControl::pausableChanged );
    connect( core, &Core::playbackSpeedChanged, this, &CoreControl::playbackSpeedChanged );
    connect( core, &Core::resettableChanged, this, &CoreControl::resettableChanged );
    connect( core, &Core::rewindableChanged, this, &CoreControl::rewindableChanged );
    connect( core, &Core::sourceChanged, this, &CoreControl::sourceChanged );
    connect( core, &Core::stateChanged, this, &CoreControl::stateChanged );
    connect( core, &Core::volumeChanged, this, &CoreControl::volumeChanged );

    // Forward these setter signals we receive to Core
    connect( this, &CoreControl::setPlaybackSpeedForwarder, core, &Core::setPlaybackSpeed );
    connect( this, &CoreControl::setSourceForwarder, core, &Core::setSource );
    connect( this, &CoreControl::setVolumeForwarder, core, &Core::setVolume );

    // Forward these method signals we receive to Core
    connect( this, &CoreControl::loadForwarder, core, &Core::load );
    connect( this, &CoreControl::playForwarder, core, &Core::play );
    connect( this, &CoreControl::pauseForwarder, core, &Core::pause );
    connect( this, &CoreControl::stopForwarder, core, &Core::stop );
    connect( this, &CoreControl::resetForwarder, core, &Core::reset );
}

void CoreControl::trackCoreStateChanges() {
    // Intercept our own signal to keep track of state changes
    connectionList << connect( this, &CoreControl::stateChanged, this, [ = ]( Control::State newState ) {
        this->state = newState;
    } );
}

void CoreControl::disconnectConnections() {
    if( connectionList.size() ) {
        qCDebug( phxControl ) << "Disconnecting" << connectionList.size() << "connection(s)";

        for( QMetaObject::Connection handle : connectionList ) {
            disconnect( handle );
        }

        connectionList.clear();
    }
}

void CoreControl::initLibretroCore() {
    /*
     * Data flow:
     * Looper/(vsync signal) >> InputManager >> LibretroCore >> [ AudioOutput, VideoOutput ]
     */

    // Make sure the last run properly cleaned up
    Q_ASSERT( !threadChildren.size() );
    Q_ASSERT( !gameThreadChildren.size() );
    Q_ASSERT( !connectionList.size() );

    // Create threads
    audioOutputThread = new QThread();
    {
        // Set names
        audioOutputThread->setObjectName( "Audio thread (libretro)" );
    }

    // Create LibretroCore
    LibretroCore *libretroCore = new LibretroCore();
    {
        core = libretroCore;
        libretroCore->setObjectName( "LibretroCore" );
        gameThreadChildren << libretroCore;

        // Connect LibretroCore to this so we can grab the current session's native framerate
        connectionList << connect( libretroCore, &LibretroCore::libretroCoreNativeFramerate, this, [ = ]( qreal hostFPS ) {
            qCDebug( phxControl ) << Q_FUNC_INFO << "hostFPS =" << hostFPS;

            if( !vsync ) {
                emit libretroSetFramerate( hostFPS );
            }
        } );

        trackCoreStateChanges();

        // Forward LibretroCore control signals as our own
        connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( stateChanged( Control::State ) ),
                 dynamic_cast<QObject *>( this ), SIGNAL( setState( Control::State ) ) );

        // Connect LibretroCore to the forwarder system
        connectCoreForwarder();

        // Intercept our own signal to handle state changes
        // In VSync mode, we have to get a chain reaction going in VideoOutput by manually asking for the first frame
        // Will only fire once per session thanks to the disconnect()
        // Credit for the std::make_shared idea: http://stackoverflow.com/a/14829520/4190028
        // TODO: Let the user set this to the true value (based on pixel clock and timing parameters/measuring)
        auto playHandlerHandle = std::make_shared<QMetaObject::Connection>();
        *playHandlerHandle = connect( this, &CoreControl::stateChanged, this, [ this, playHandlerHandle ]( Control::State newState ) {
            if( ( Control::State )newState == Control::PLAYING ) {
                disconnect( *playHandlerHandle );

                if( vsync ) {
                    emit libretroSetFramerate( 60.0 );
                    emit libretroCoreDoFrame();
                }
            }
        } );
    }

    // Create Looper
    looper = new Looper();
    {
        looper->setObjectName( "Looper (Libretro)" );
        gameThreadChildren << looper;

        // Connect control and framerate assigning signals to Looper
        if( !vsync ) {
            CONNECT_CONTROL_CONTROLLABLE( this, looper );
            connect( this, &CoreControl::libretroSetFramerate, looper, &Looper::libretroSetFramerate );
        }

        // InputManager (Producer)

        // inputManager is a property should've been set by this point
        // Belongs to the QML thread
        Q_ASSERT( inputManager );
        // Connect InputManager to LibretroCore (produces input data which also drives frame production in LibretroCore)
        CONNECT_PRODUCER_CONSUMER( inputManager, libretroCore );

        // Connect control signals to InputManager
        CONNECT_CONTROL_CONTROLLABLE( this, inputManager );

        // Connect Looper to InputManager (drive input polling)
        if( !vsync ) {
            connect( looper, &Looper::timeout, inputManager, &InputManager::libretroGetInputState );
        }
    }

    // Create the consumers
    audioOutput = new AudioOutput();
    {
        audioOutput->moveToThread( audioOutputThread );
        audioOutput->setObjectName( "AudioOutput (Libretro)" );
        threadChildren[ audioOutputThread ] << audioOutput ;
        // videoOutput is a property should've been set by this point
        // Belongs to the QML thread
        Q_ASSERT( videoOutput );

        // Connect LibretroCore to the consumers (AV output)
        CONNECT_PRODUCER_CONSUMER( libretroCore, audioOutput );
        CONNECT_PRODUCER_CONSUMER( libretroCore, videoOutput );

        // Connect control signals to consumers
        CONNECT_CONTROL_CONTROLLABLE( this, audioOutput );
        CONNECT_CONTROL_CONTROLLABLE( this, videoOutput );

        // Connect framerate assigning signal to AudioOutput
        connect( this, &CoreControl::libretroSetFramerate, audioOutput, &AudioOutput::libretroSetFramerate );

        // Connect VideoOutput, this to InputManager (drive input polling)
        if( vsync ) {
            connect( videoOutput, &VideoOutput::windowUpdate, inputManager, &InputManager::libretroGetInputState );
            connect( this, &CoreControl::libretroCoreDoFrame, inputManager, &InputManager::libretroGetInputState );
        }
    }

    // Set up cleanup stuff
    {
        // Intercept our own signal to handle state changes
        // Once LibretroCore has fully stopped, delete all threads and all objects declared up above
        auto stoppedHandlerHandle = std::make_shared<QMetaObject::Connection>();
        *stoppedHandlerHandle = connect( this, &CoreControl::stateChanged, this, [ this, stoppedHandlerHandle ]( Control::State newState ) {
            if( ( Control::State )newState == Control::STOPPED ) {
                disconnect( *stoppedHandlerHandle );
                cleanup();
            }
        } );
    }

    // Start threads
    audioOutputThread->start();

}
