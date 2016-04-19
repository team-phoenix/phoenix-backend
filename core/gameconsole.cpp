#include "gameconsole.h"

#include "controllable.h"

// Timers that drive frame production (controllers)
#include "looper.h"

// Producers
#include "producer.h"
#include "core.h"
#include "libretrocore.h"

// Consumers
#include "consumer.h"
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
    : QObject( parent ), Control()
{

}

// Public slots

void GameConsole::shutdown() {
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

void GameConsole::setSrc(QVariant _src) {
    m_src = _src;
    QMetaObject::invokeMethod( (QObject *)core, "setSrc", Q_ARG( QVariant, _src) );
}

void GameConsole::setVideoOutput( VideoOutput *videoOutput ) {
    // This is relevant to us only, so we'll immediately acknowledge
    this->videoOutput = videoOutput;
    emit videoOutputChanged( videoOutput );
}

void GameConsole::setPlaybackSpeed( qreal playbackSpeed ) {
    emit setPlaybackSpeedForwarder( playbackSpeed );
}

void GameConsole::setVolume( qreal volume ) {
    emit setVolumeForwarder( volume );
}

void GameConsole::setVsync( bool vsync ) {
    // This is relevant to us only, so we'll immediately acknowledge
    this->vsync = vsync;
    emit vsyncChanged( vsync );
}

void GameConsole::load() {
    if( core ) {
        stop();
    }

    initLibretroCore();
    qCDebug( phxControl ) << "LibretroCore fully initalized and connected";

    setSrc( m_src );
    QMetaObject::invokeMethod( core, "load" );

    // Determine Core type, instantiate appropiate type
//    if( source[ QStringLiteral( "type" ) ] == QStringLiteral( "libretro" ) ) {
//        initLibretroCore();
//    } else {
//        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
//                                           << source[ "type" ] << QStringLiteral( " passed to load()!" );
//        return;
//    }

    //emit setSourceForwarder( source );

    //emit loadForwarder();
}

void GameConsole::play() {
    QMetaObject::invokeMethod( core, "play" );
}

void GameConsole::pause() {
    QMetaObject::invokeMethod( core, "pause" );
}

void GameConsole::stop() {
    QMetaObject::invokeMethod( core, "stop" );
}

void GameConsole::reset() {
    QMetaObject::invokeMethod( core, "reset" );
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
    connectionList << connect( core, &Core::pausableChanged, this, &GameConsole::pausableChanged );
    connectionList << connect( core, &Core::playbackSpeedChanged, this, &GameConsole::playbackSpeedChanged );
    connectionList << connect( core, &Core::resettableChanged, this, &GameConsole::resettableChanged );
    connectionList << connect( core, &Core::rewindableChanged, this, &GameConsole::rewindableChanged );
    //connectionList << connect( core, &Core::sourceChanged, this, &GameConsole::sourceChanged );
    connectionList << connect( core, &Core::stateChanged, this, &GameConsole::stateChanged );
    connectionList << connect( core, &Core::volumeChanged, this, &GameConsole::volumeChanged );

    // Forward these setter signals we receive to Core
    connectionList << connect( this, &GameConsole::setPlaybackSpeedForwarder, core, &Core::setPlaybackSpeed );
    //connectionList << connect( this, &GameConsole::setSourceForwarder, core, &Core::setSource );
    connectionList << connect( this, &GameConsole::setVolumeForwarder, core, &Core::setVolume );

}

void GameConsole::trackCoreStateChanges() {
    // Intercept our own signal to keep track of state changes
    connectionList << connect( this, &GameConsole::stateChanged, this, [ = ]( Control::State newState ) {
        this->state = newState;
    } );
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
    Q_ASSERT( !connectionList.size() );
    Q_ASSERT( !threadChildren.size() );
    Q_ASSERT( !gameThreadChildren.size() );

    // Create threads
    audioOutputThread = new QThread;
    {
        // Set names
        audioOutputThread->setObjectName( "Audio thread (libretro)" );
        pointersToClear << ( QObject ** )&audioOutputThread;
    }

    // Create LibretroCore
    auto *libretroCore = new LibretroCore;
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
        *playHandlerHandle = connect( this, &GameConsole::stateChanged, this, [ this, playHandlerHandle ]( Control::State newState ) {
            if( ( Control::State )newState == Control::PLAYING ) {
                disconnect( *playHandlerHandle );

                if( vsync ) {
                    emit libretroSetFramerate( 60.0 );
                }
            }
        });
    }

    // Create Looper
    looper = new Looper;
    {
        pointersToClear << ( QObject ** )&looper;
        looper->setObjectName( "Looper (Libretro)" );
        gameThreadChildren << looper;

        // Connect control and framerate assigning signals to Looper
        if( !vsync ) {
            CLIST_CONNECT_CONTROL_CONTROLLABLE( this, looper );
            connectionList << connect( this, &GameConsole::libretroSetFramerate, looper, &Looper::libretroSetFramerate );
        }
    }

    // GamepadManager (Producer)
    {

         //Connect GamepadManager to LibretroCore (produces input data which also drives frame production in LibretroCore)
         CLIST_CONNECT_PRODUCER_CONSUMER( &m_gamepadManager, libretroCore );

         CLIST_CONNECT_CONTROL_CONTROLLABLE( this, &m_gamepadManager );

         //Connect Looper to GamepadManager (drive input polling)
        if( !vsync ) {
            connectionList << connect( looper, &Looper::timeout, &m_gamepadManager, &GamepadManager::poll );
            connectionList << connect( this, &GameConsole::gamepadAdded, &m_gamepadManager, &GamepadManager::gamepadAdded );
            connectionList << connect( this, &GameConsole::gamepadRemoved, &m_gamepadManager, &GamepadManager::gamepadRemoved);

        }
    }

    // Create the consumers
    audioOutput = new AudioOutput;
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
        connectionList << connect( this, &GameConsole::libretroSetFramerate, audioOutput, &AudioOutput::libretroSetFramerate );

        // Connect VideoOutput, this to GamepadManager (drive input polling)
        // TODO: Don't hook window updates, instead create a VSynced timer object and hook that
        if( vsync ) {
            connectionList << connect( videoOutput, &VideoOutput::windowUpdate, &m_gamepadManager, &GamepadManager::poll );
            connectionList << connect( this, &GameConsole::libretroCoreDoFrame, &m_gamepadManager, &GamepadManager::poll );
        }

        // Force a frame if we're playing and vsync mode is on (this starts a chain reaction in VideoOutput)
        connectionList << connect( this, &GameConsole::stateChanged, this, [ this ]( Control::State newState ) {
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
        *stoppedHandlerHandle = connect( this, &GameConsole::stateChanged, this, [ this, stoppedHandlerHandle ]( Control::State newState ) {
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

    qCDebug( phxControl ) << &m_gamepadManager << m_gamepadManager.thread();
    qCDebug( phxControl ) << videoOutput << videoOutput->thread();
    qCDebug( phxControl ) << this << this->thread();
    qCDebug( phxControl ) << "Executing from:" << QThread::currentThread();

    // Start threads
    audioOutputThread->start();
}
