#include "corecontrol.h"

// Public

CoreControl::CoreControl( QObject *parent ) : QObject( parent ), Control(),
    threadChildren(),
    gameThreadChildren(),
    pointersToClear(),
    microTimer( nullptr ),
    inputManager( nullptr ),
    core( nullptr ),
    audioOutput( nullptr ),
    audioOutputThread( nullptr ),
    videoOutput( nullptr ),
    vsync( false ),
    connectionList(),
    source() {

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

    QThread::currentThread()->quit();
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
    // Store this, we'll pass it to Core once it's time to load (also Core doesn't exist yet)
    this->source = source;
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
    if( core ) {
        stop();
    }

    // Determine Core type, instantiate appropiate type
    if( source[ QStringLiteral( "type" ) ] == QStringLiteral( "libretro" ) ) {
        initLibretroCore();
        // qCDebug( phxControl ) << "LibretroCore fully initalized and connected";
    } else {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
                                           << source[ "type" ] << QStringLiteral( " passed to load()!" );
        return;
    }

    emit setSourceForwarder( source );
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

void CoreControl::zeroPointers() {
    if( pointersToClear.size() ) {
        for( QObject **pointer : pointersToClear ) {
            *pointer = nullptr;
        }
    }
}

void CoreControl::deleteThreads() {

    if( threadChildren.size() ) {
        qCDebug( phxControl ) << "Shutting down" << threadChildren.size() << "thread(s)";

        for( QThread *thread_ : threadChildren.keys() ) {
            qCDebug( phxControl ).nospace() << "Shutting down thread: " << thread_;

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
        qCDebug( phxControl ) << "Shutting down" << gameThreadChildren.size() << "game thread children";

        for( QObject *obj : gameThreadChildren ) {
            qCDebug( phxControl ) << "    Shutting down:" << obj;
            Q_ASSERT( obj->thread() == thread() );
            delete obj;
        }

        gameThreadChildren.clear();
    } else {
        qCDebug( phxControl ) << "No children in game thread to delete, continuing";
    }
}

void CoreControl::cleanup() {
    disconnectConnections();
    deleteThreads();
    deleteGameThreadChildren();
    zeroPointers();
    qCDebug( phxControl ) << "Fully unloaded";
}

void CoreControl::connectCoreForwarder() {
    // Forward these signals that are important to QML (but not us) to whoever's concerned (such as Core)
    // In some rare cases we DO hook our own signal to do stuff, but not generally
    connectionList << connect( core, &Core::pausableChanged, this, &CoreControl::pausableChanged );
    connectionList << connect( core, &Core::playbackSpeedChanged, this, &CoreControl::playbackSpeedChanged );
    connectionList << connect( core, &Core::resettableChanged, this, &CoreControl::resettableChanged );
    connectionList << connect( core, &Core::rewindableChanged, this, &CoreControl::rewindableChanged );
    connectionList << connect( core, &Core::sourceChanged, this, &CoreControl::sourceChanged );
    connectionList << connect( core, &Core::stateChanged, this, &CoreControl::stateChanged );
    connectionList << connect( core, &Core::volumeChanged, this, &CoreControl::volumeChanged );

    // Forward these setter signals we receive to Core
    connectionList << connect( this, &CoreControl::setPlaybackSpeedForwarder, core, &Core::setPlaybackSpeed );
    connectionList << connect( this, &CoreControl::setSourceForwarder, core, &Core::setSource );
    connectionList << connect( this, &CoreControl::setVolumeForwarder, core, &Core::setVolume );

    // Forward these method signals we receive to Core
    connectionList << connect( this, &CoreControl::loadForwarder, core, &Core::load );
    connectionList << connect( this, &CoreControl::playForwarder, core, &Core::play );
    connectionList << connect( this, &CoreControl::pauseForwarder, core, &Core::pause );
    connectionList << connect( this, &CoreControl::stopForwarder, core, &Core::stop );
    connectionList << connect( this, &CoreControl::resetForwarder, core, &Core::reset );
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
    } else {
        qCDebug( phxControl ) << "No connections to disconnect, continuing";
    }
}

// Private (Core loaders)

void CoreControl::initLibretroCore() {
    /*
     * Pipeline:
     * Looper/(vsync signal) >> InputManager >> LibretroCore >> [ AudioOutput, VideoOutput ]
     *
     * Special considerations:
     * If VSync is off:
     * Libretro cores have native framerates (coreFPS), but we may drive frame production at a different rate (hostFPS).
     * LibretroCore has a signal called libretroCoreNativeFramerate() which we hook up to Looper and AudioOutput to
     * ensure they're aware of this framerate so they may use it. This signal gets emitted right after emitting
     * producerFormat(). In other words, when VSync is off, make hostFPS = coreFPS.
     *
     * If VSync is on, we instead bootstrap continuous window updates once we go to PLAYING by
     * manually invoking the first frame. VideoOutput will then provide the libretroCoreDoFrame() signals from there on.
     * TODO: Don't hook window updates, instead create a VSynced timer object and hook that
     */

    // Make sure the last run properly cleaned up
    Q_ASSERT( !connectionList.size() );
    Q_ASSERT( !threadChildren.size() );
    Q_ASSERT( !gameThreadChildren.size() );

    // Create threads
    audioOutputThread = new QThread();
    {
        // Set names
        audioOutputThread->setObjectName( "Audio thread (libretro)" );
        pointersToClear << ( QObject ** )&audioOutputThread;
    }

    // Create LibretroCore
    LibretroCore *libretroCore = new LibretroCore();
    {
        core = libretroCore;
        pointersToClear << ( QObject ** )&core;
        libretroCore->setObjectName( "LibretroCore" );
        gameThreadChildren << libretroCore;

        // Connect LibretroCore to this so we can grab the current session's native framerate
        connectionList << connect( libretroCore, &LibretroCore::libretroCoreNativeFramerate, this, [ = ]( qreal hostFPS ) {
            qCDebug( phxControl ) << Q_FUNC_INFO << "hostFPS =" << hostFPS;

            if( !vsync ) {
                emit libretroSetFramerate( hostFPS );
            } else {
                qCDebug( phxControl ) << "VSync mode on, ignoring coreFPS value";
            }
        } );

        trackCoreStateChanges();

        // Forward LibretroCore control signals as our own
        connectionList << connect( dynamic_cast<QObject *>( libretroCore ), SIGNAL( stateChanged( Control::State ) ),
                                   dynamic_cast<QObject *>( this ), SIGNAL( setState( Control::State ) ) );

        // Connect LibretroCore to the forwarder system
        connectCoreForwarder();

        // Intercept our own signal to handle state changes
        // Inform consumers of vsync rate once we start playing
        // Will only fire once per session thanks to the disconnect()
        // Credit for the std::make_shared idea: http://stackoverflow.com/a/14829520/4190028
        // TODO: Let the user set this to the true value (based on pixel clock and timing parameters/measuring)
        auto playHandlerHandle = std::make_shared<QMetaObject::Connection>();
        *playHandlerHandle = connect( this, &CoreControl::stateChanged, this, [ this, playHandlerHandle ]( Control::State newState ) {
            if( ( Control::State )newState == Control::PLAYING ) {
                disconnect( *playHandlerHandle );

                if( vsync ) {
                    emit libretroSetFramerate( 60.0 );
                }
            }
        } );
    }

    // Create MicroTimer
    microTimer = new MicroTimer();
    {
        pointersToClear << ( QObject ** )&microTimer;
        microTimer->setObjectName( "MicroTimer (Libretro)" );
        gameThreadChildren << microTimer;

        // Connect control and framerate assigning signals to Looper
        if( !vsync ) {
            CLIST_CONNECT_CONTROL_CONTROLLABLE( this, microTimer );
            connectionList << connect( this, &CoreControl::libretroSetFramerate, [ this ]( qreal hostFPS ) {
                microTimer->setProperty( "frequency", hostFPS );
            } );
        }
    }

    // InputManager (Producer)
    {
        // inputManager is a property should've been set by this point
        // Belongs to the QML thread
        Q_ASSERT( inputManager );

        // Connect InputManager to LibretroCore (produces input data which also drives frame production in LibretroCore)
        CLIST_CONNECT_PRODUCER_CONSUMER( inputManager, libretroCore );

        // Connect control signals to InputManager
        CLIST_CONNECT_CONTROL_CONTROLLABLE( this, inputManager );

        // Connect Looper to InputManager (drive input polling)
        if( !vsync ) {
            connectionList << connect( microTimer, &MicroTimer::timeout, inputManager, &InputManager::libretroGetInputStateNoTimestamp );
        }
    }

    // Create the consumers
    audioOutput = new AudioOutput();
    {
        pointersToClear << ( QObject ** )&audioOutput;
        audioOutput->moveToThread( audioOutputThread );
        audioOutput->setObjectName( "AudioOutput (Libretro)" );
        threadChildren[ audioOutputThread ] << audioOutput;

        // videoOutput is a property should've been set by this point
        // Belongs to the QML thread
        Q_ASSERT( videoOutput );

        // Connect LibretroCore to the consumers (AV output)
        CLIST_CONNECT_PRODUCER_CONSUMER( libretroCore, audioOutput );
        CLIST_CONNECT_PRODUCER_CONSUMER( libretroCore, videoOutput );

        // Connect control signals to consumers
        CLIST_CONNECT_CONTROL_CONTROLLABLE( this, audioOutput );
        CLIST_CONNECT_CONTROL_CONTROLLABLE( this, videoOutput );

        // Connect framerate assigning signal to AudioOutput
        connectionList << connect( this, &CoreControl::libretroSetFramerate, audioOutput, &AudioOutput::libretroSetFramerate );

        // Connect VideoOutput, this to InputManager (drive input polling)
        // TODO: Don't hook window updates, instead create a VSynced timer object and hook that
        if( vsync ) {
            connectionList << connect( videoOutput, &VideoOutput::windowUpdate, inputManager, &InputManager::libretroGetInputState );
            connectionList << connect( this, &CoreControl::libretroCoreDoFrame, inputManager, &InputManager::libretroGetInputState );
        }

        // Force a frame if we're playing and vsync mode is on (this starts a chain reaction in VideoOutput)
        connectionList << connect( this, &CoreControl::stateChanged, this, [ this ]( Control::State newState ) {
            if( ( Control::State )newState == Control::PLAYING ) {
                if( vsync ) {
                    emit libretroCoreDoFrame( QDateTime::currentMSecsSinceEpoch() );
                }
            }
        } );
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

    // Print all children and their threads
    for( auto childList : threadChildren ) {
        for( auto child : childList ) {
            qCDebug( phxControl ) << child << child->thread();
        }
    }

    for( auto child : gameThreadChildren ) {
        qCDebug( phxControl ) << child << child->thread();
    }

    qCDebug( phxControl ) << inputManager << inputManager->thread();
    qCDebug( phxControl ) << videoOutput << videoOutput->thread();
    qCDebug( phxControl ) << this << this->thread();
    qCDebug( phxControl ) << "Executing from:" << QThread::currentThread();

    // Start threads
    audioOutputThread->start();
}
