#include "gameconsole.h"


// Timers that drive frame production (controllers)
#include "looper.h"

// Producers
#include "core.h"
#include "libretrocore.h"

// Consumers
#include "audiooutput.h"
#include "videooutput.h"
#include "gamepad.h"
#include "keyboard.h"

// Misc
#include "logging.h"

// Public
#include <QThread>
#include <QDateTime>
#include <memory>

using namespace Input;

GameConsole::GameConsole( QObject *parent )
    : QObject( parent ) {

}

void GameConsole::componentComplete() {
    initLibretroCore();
}

// Public slots

void GameConsole::shutdown() {
    //    if( state != Control::STOPPED && m_core ) {
    //        qCDebug( phxControl ) << "Stopping running game...";
    //        stop();
    //    } else if( !m_core ) {
    //        qCDebug( phxControl ) << "No core running, continuing";
    //        cleanup();
    //    } else {
    //        qCDebug( phxControl ) << "Core has already stopped, continuing";
    //        cleanup();
    //    }
    stop();
    cleanup();
    QThread::currentThread()->quit();
}

void GameConsole::setSource( QVariantMap source ) {
    this->source = source;
    // Do not send the source to the pipeline until load() is called
}

void GameConsole::setVideoOutput( VideoOutput *videoOutput ) {
    m_videoOutput = videoOutput;
}

void GameConsole::setPlaybackSpeed( qreal playbackSpeed ) {

    emit setPlaybackSpeedForwarder( playbackSpeed );
}

void GameConsole::setVolume( qreal volume ) {
    emit setVolumeForwarder( volume );
}

void GameConsole::setVsync( bool vsync ) {
    // This is relevant to us only, so we'll immediately acknowledge
    // FIXME: Pass to pipeline
    m_vsync = vsync;
    emit vsyncChanged( vsync );
}

void GameConsole::load() {
    if( m_core ) {
        stop();
    }

    // Determine Core type, instantiate appropiate type
    if( source[ QStringLiteral( "type" ) ] == QStringLiteral( "libretro" ) ) {
        initLibretroCore();
        qCDebug( phxControl ) << "LibretroCore fully initalized and connected";
    } else {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
                                           << source[ "type" ] << QStringLiteral( " passed to load()!" );
        return;
    }

    Q_ASSERT( pipelineHead );
    QMetaObject::invokeMethod( pipelineHead, "controlIn", Q_ARG( Command , Command::Set_Source_QVariantMap )
                               , Q_ARG( QVariant, QVariant( source ) ) );
    QMetaObject::invokeMethod( pipelineHead, "stateIn", Q_ARG( PipelineState, PipelineState::Loading ) );

    //emit setSourceForwarder( source );

    //emit loadForwarder();
}

void GameConsole::play() {
    Q_ASSERT( pipelineHead );
    QMetaObject::invokeMethod( pipelineHead, "stateIn", Q_ARG( PipelineState, PipelineState::Playing ) );
}

void GameConsole::pause() {
    Q_ASSERT( pipelineHead );
    QMetaObject::invokeMethod( pipelineHead, "stateIn", Q_ARG( PipelineState, PipelineState::Paused ) );
}

void GameConsole::stop() {
    Q_ASSERT( pipelineHead );
    QMetaObject::invokeMethod( pipelineHead, "stateIn", Q_ARG( PipelineState, PipelineState::Stopped ) );
}

void GameConsole::reset() {
    Q_ASSERT( pipelineHead );
    //QMetaObject::invokeMethod( looper, "controlIn", Q_ARG( Command, Command::State_Changed_To_Playing ) );
}

// Private

void GameConsole::zeroPointers() {
    if( pointersToClear.size() ) {
        for( QObject **pointer : pointersToClear ) {
            *pointer = nullptr;
        }
    }
}

void GameConsole::deleteThreads() {

    if( threadChildren.size() ) {
        qCDebug( phxControl ) << "Shutting down" << threadChildren.size() << "thread(s)";

        for( QThread *_thread : threadChildren.keys() ) {
            qCDebug( phxControl ).nospace() << "Shutting down thread: " << _thread;

            // No harm in connecting right before the thread dies!
            for( QObject *obj : threadChildren[ _thread ] ) {
                qCDebug( phxControl ) << "    Shutting down:" << obj;
                Q_ASSERT( obj->thread() != thread() );
                connect( _thread, &QThread::finished, obj, &QObject::deleteLater );
            }

            // Terminate the thread
            _thread->quit();
            _thread->wait();
            _thread->deleteLater();
        }

        threadChildren.clear();
    } else {
        qCDebug( phxControl ) << "No threads to delete, continuing";
    }
}

void GameConsole::deleteGameThreadChildren() {
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

void GameConsole::cleanup() {
    disconnectConnections();
    deleteThreads();
    deleteGameThreadChildren();
    zeroPointers();
    qCDebug( phxControl ) << "Fully unloaded";
}

void GameConsole::connectCoreForwarder() {
    // Forward these signals that are important to QML (but not us) to whoever's concerned (such as Core)
    // In some rare cases we DO hook our own signal to do stuff, but not generally
    // FIXME: This should be taken over by MetaOutput
    connectionList << connect( m_core, &Core::pausableChanged, this, &GameConsole::pausableChanged );
    connectionList << connect( m_core, &Core::playbackSpeedChanged, this, &GameConsole::playbackSpeedChanged );
    connectionList << connect( m_core, &Core::resettableChanged, this, &GameConsole::resettableChanged );
    connectionList << connect( m_core, &Core::rewindableChanged, this, &GameConsole::rewindableChanged );
    connectionList << connect( m_core, &Core::volumeChanged, this, &GameConsole::volumeChanged );

    // Forward these setter signals we receive to Core
    connectionList << connect( this, &GameConsole::setPlaybackSpeedForwarder, m_core, &Core::setPlaybackSpeed );
    connectionList << connect( this, &GameConsole::setVolumeForwarder, m_core, &Core::setVolume );

}

void GameConsole::disconnectConnections() {
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

void GameConsole::initLibretroCore() {
    /*
     * Pipeline:
     * Looper/(vsync signal) >> GamepadManager >> LibretroCore >> [ AudioOutput, VideoOutput ]
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
    Q_ASSERT( connectionList.isEmpty() );
    Q_ASSERT( threadChildren.isEmpty() );
    Q_ASSERT( gameThreadChildren.isEmpty() );

    // Create threads
    m_audioOutputThread = new QThread;
    {
        // Set names
        m_audioOutputThread->setObjectName( "Audio thread (libretro)" );
        pointersToClear << ( QObject ** )&m_audioOutputThread;
    }

    // Create LibretroCore
    auto *libretroCore = new LibretroCore;
    {
        m_core = libretroCore;

        pointersToClear << ( QObject ** )&m_core;
        libretroCore->setObjectName( "LibretroCore" );
        gameThreadChildren << libretroCore;

        // Connect LibretroCore to this so we can grab the current session's native framerate
        connectionList << connect( libretroCore, &LibretroCore::libretroCoreNativeFramerate, this, [this]( qreal hostFPS ) {
            qCDebug( phxControl ) << Q_FUNC_INFO << "hostFPS =" << hostFPS;

            if( !m_vsync ) {
                emit libretroSetFramerate( hostFPS );
            } else {
                qCDebug( phxControl ) << "VSync mode on, ignoring coreFPS value";
            }
        } );


        // Connect LibretroCore to the forwarder system
        //connectCoreForwarder();

        // Intercept our own signal to handle state changes
        // Inform consumers of vsync rate once we start playing
        // Will only fire once per session thanks to the disconnect()
        // Credit for the std::make_shared idea: http://stackoverflow.com/a/14829520/4190028
        // TODO: Let the user set this to the true value (based on pixel clock and timing parameters/measuring)
        //        auto playHandlerHandle = std::make_shared<QMetaObject::Connection>();
        //        *playHandlerHandle = connect( this, &GameConsole::stateChanged, this, [ this, playHandlerHandle ]( Control::State newState ) {
        //            if( ( Control::State )newState == Control::PLAYING ) {
        //                disconnect( *playHandlerHandle );

        //                if( m_vsync ) {
        //                    emit libretroSetFramerate( 60.0 );
        //                }
        //            }
        //        });
    }

    // Create Looper
    m_looper = new Looper;
    {
        pointersToClear << ( QObject ** )&m_looper;
        m_looper->setObjectName( "Looper (Libretro)" );
        gameThreadChildren << m_looper;

        // Connect control and framerate assigning signals to Looper
        if( !m_vsync ) {
            //CLIST_CONNECT_CONTROL_CONTROLLABLE( this, m_looper );


            connectionList << connect( this, &GameConsole::libretroSetFramerate, m_looper, &Looper::libretroSetFramerate );
        }

        // Mark as root of pipeline
        pipelineHead = m_looper;
        pointersToClear << &pipelineHead;
    }

    // GamepadManager (Producer)
    {

        //Connect GamepadManager to LibretroCore (produces input data which also drives frame production in LibretroCore)
        //         CLIST_CONNECT_PRODUCER_CONSUMER( &m_gamepadManager, libretroCore );

        //         CLIST_CONNECT_CONTROL_CONTROLLABLE( this, &m_gamepadManager );

        //Connect Looper to GamepadManager (drive input polling)
        if( !m_vsync ) {
            //connectionList << connect( looper, &Looper::timeout, &m_gamepadManager, &GamepadManager::poll );


        }
    }

    // Create the consumers
    m_audioOutput = new AudioOutput;
    {
        pointersToClear << ( QObject ** )&m_audioOutput;
        m_audioOutput->moveToThread( m_audioOutputThread );
        m_audioOutput->setObjectName( "AudioOutput (Libretro)" );
        threadChildren[ m_audioOutputThread ] << m_audioOutput;

        // videoOutput is a property should've been set by this point
        // Belongs to the QML thread
        Q_ASSERT( m_videoOutput );

        // Connect LibretroCore to the consumers (AV output)
        //        CLIST_CONNECT_PRODUCER_CONSUMER( libretroCore, audioOutput );
        //        CLIST_CONNECT_PRODUCER_CONSUMER( libretroCore, videoOutput );

        // Connect control signals to consumers
        //        CLIST_CONNECT_CONTROL_CONTROLLABLE( this, audioOutput );
        //        CLIST_CONNECT_CONTROL_CONTROLLABLE( this, videoOutput );

        // Connect framerate assigning signal to AudioOutput
        connectionList << connect( this, &GameConsole::libretroSetFramerate, m_audioOutput, &AudioOutput::libretroSetFramerate );

        // Connect VideoOutput, this to GamepadManager (drive input polling)
        // TODO: Don't hook window updates, instead create a VSynced timer object and hook that
        if( m_vsync ) {
            //connectionList << connect( videoOutput, &VideoOutput::windowUpdate, &m_gamepadManager, &GamepadManager::poll );
            // connectionList << connect( this, &GameConsole::libretroCoreDoFrame, &m_gamepadManager, &GamepadManager::poll );
        }

    }

    // Set up cleanup stuff
    {
        // Intercept our own signal to handle state changes
        // Once LibretroCore has fully stopped, delete all threads and all objects declared up above
        //        auto stoppedHandlerHandle = std::make_shared<QMetaObject::Connection>();
        //        *stoppedHandlerHandle = connect( this, &GameConsole::stateChanged, this, [ this, stoppedHandlerHandle ]( Control::State newState ) {
        //            if( ( Control::State )newState == Control::STOPPED ) {
        //                disconnect( *stoppedHandlerHandle );
        //                cleanup();
        //            }
        //        } );
    }

    connectionList << connect( &m_gamepadManager, &GamepadManager::gamepadAdded, this, &GameConsole::gamepadAdded );
    connectionList << connect( &m_gamepadManager, &GamepadManager::gamepadRemoved, this, &GameConsole::gamepadRemoved );

    connectionList << connectInterface( m_looper, &m_gamepadManager );
    connectionList << connectInterface( &m_gamepadManager, libretroCore );
    connectionList << connectInterface( libretroCore, m_audioOutput );
    Q_ASSERT( m_videoOutput );
    connectionList << connectInterface( libretroCore, m_videoOutput );

    // Print all children and their threads
    for( auto childList : threadChildren ) {
        for( auto child : childList ) {
            qCDebug( phxControl ) << child << child->thread();
        }
    }

    for( auto child : gameThreadChildren ) {
        qCDebug( phxControl ) << child << child->thread();
    }

    qCDebug( phxControl ) << &m_gamepadManager << m_gamepadManager.thread();
    qCDebug( phxControl ) << m_videoOutput << m_videoOutput->thread();
    qCDebug( phxControl ) << this << this->thread();
    qCDebug( phxControl ) << "Executing from:" << QThread::currentThread();

    // Start threads
    m_audioOutputThread->start();
}
