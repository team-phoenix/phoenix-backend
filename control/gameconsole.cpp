#include "gameconsole.h"

#include "logging.h"

GameConsole::GameConsole( Node *parent ) : Node( parent ),
    gameThread( new QThread() ),
    audioOutput( new AudioOutput() ),
    gamepadManager( new GamepadManager() ),
    libretroCore( new LibretroCore() ),
    microTimer( new MicroTimer() ),
    remapper( new Remapper() ) {
    // Move all our stuff to the game thread
    //audioOutput->moveToThread( gameThread );
    //gamepadManager->moveToThread( gameThread );
    //libretroCore->moveToThread( gameThread );
    //microTimer->moveToThread( gameThread );
    //remapper->moveToThread( gameThread );

    gameThread->setObjectName( "Game thread" );
    gameThread->start();

    // Connect global pipeline
    connectNodes( this, microTimer );
    connectNodes( microTimer, gamepadManager );
    connectNodes( gamepadManager, remapper );

    // Connect GlobalGamepad (which lives in QML) to the global pipeline as soon as it's set
    connect( this, &GameConsole::globalGamepadChanged, this, [ = ]() {
        if( globalGamepad ) {
            connectNodes( remapper, globalGamepad );
        }
    } );

    // Begin timer so it may poll for input
    emit controlOut( Command::HeartbeatRate, 60.0, QDateTime::currentMSecsSinceEpoch() );

    // Handle app quitting
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ = ]() {
        qDebug() << "";
        qCInfo( phxControl ) << ">>>>>>>> User requested app to close, shutting down (waiting up to 30 seconds)...";
        qDebug() << "";

        // Tell the pipeline to stop then quit
        quitFlag = true;
        emit controlOut( Command::KillTimer, QVariant(), QDateTime::currentMSecsSinceEpoch() );
        emit controlOut( Command::Stop, QVariant(), QDateTime::currentMSecsSinceEpoch() );

        // Wait up to 30 seconds to let the pipeline finish its events
        gameThread->wait( 30 * 1000 );
        gameThread->deleteLater();

        // Destroy our global pipeline objects
        audioOutput->deleteLater();
        gamepadManager->deleteLater();
        libretroCore->deleteLater();
        microTimer->deleteLater();
        remapper->deleteLater();

        qDebug() << "";
        qCInfo( phxControl ) << ">>>>>>>> Fully unloaded, quitting!";
        qDebug() << "";
    } );
}

GameConsole::~GameConsole() {
}

void GameConsole::classBegin() {
}

void GameConsole::componentComplete() {
}

// Public slots

void GameConsole::load() {
    if( source[ "type" ] == QStringLiteral( "libretro" ) ) {
        loadLibretro();
        emit controlOut( Command::SetSource, source, QDateTime::currentMSecsSinceEpoch() );
        emit controlOut( Command::Load, QVariant(), QDateTime::currentMSecsSinceEpoch() );
    } else if( source[ "type" ].toString().isEmpty() ) {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Source was not set!" );
    } else {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
                                           << source[ "type" ] << QStringLiteral( " passed to load()!" );
    }
}

void GameConsole::play() {
    emit controlOut( Command::Play, QVariant(), QDateTime::currentMSecsSinceEpoch() );
}

void GameConsole::pause() {
    emit controlOut( Command::Pause, QVariant(), QDateTime::currentMSecsSinceEpoch() );
}

void GameConsole::stop() {
    emit controlOut( Command::Stop, QVariant(), QDateTime::currentMSecsSinceEpoch() );
}

void GameConsole::reset() {
    emit controlOut( Command::Reset, QVariant(), QDateTime::currentMSecsSinceEpoch() );
}

void GameConsole::unload() {
    if( source[ "type" ] == QStringLiteral( "libretro" ) ) {
        unloadLibretro();
    } else if( source[ "type" ].toString().isEmpty() ) {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Source was not set!" );
    } else {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
                                           << source[ "type" ] << QStringLiteral( " passed to unload()!" );
    }
}

// Private

void GameConsole::loadLibretro() {
    // Ensure that the properties were set in QML
    Q_ASSERT_X( controlOutput, "libretro load", "controlOutput was not set!" );
    Q_ASSERT_X( videoOutput, "libretro load", "videoOutput was not set!" );

    // Connect LibretroCore to the global pipeline
    sessionConnections << connectNodes( remapper, libretroCore );

    // Connect LibretroCore to its children
    sessionConnections << connectNodes( libretroCore, controlOutput );
    //sessionConnections << connectNodes( libretroCore, audioOutput );
    //sessionConnections << connectNodes( libretroCore, videoOutput );

    // Hook controlOutput so we know when Command::Stop has reached LibretroCore
    // FIXME: Rework the design so this doesn't have to be done this way
    sessionConnections << connect( controlOutput, &ControlOutput::controlOut, controlOutput, [ & ]( Command command, QVariant, qint64 ) {
        switch( command ) {
            case Command::Stop: {
                unloadLibretro();
                break;
            }

            default: {
                break;
            }
        }
    } );
}

void GameConsole::unloadLibretro() {
    qCDebug( phxControl ) << Q_FUNC_INFO;
    for( QMetaObject::Connection connection : sessionConnections ) {
        disconnect( connection );
    }

    sessionConnections.clear();

    if( quitFlag ) {
        gameThread->quit();
    }
}

bool GameConsole::getPausable() {
    return pausable;
}

qreal GameConsole::getPlaybackSpeed() {
    return playbackSpeed;
}

void GameConsole::setPlaybackSpeed( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged();
}

bool GameConsole::getResettable() {
    return resettable;
}

bool GameConsole::getRewindable() {
    return rewindable;
}

QVariantMap GameConsole::getSource() {
    return source;
}

void GameConsole::setSource( QVariantMap source ) {
    this->source = source;
    emit controlOut( Command::SetSource, source, QDateTime::currentMSecsSinceEpoch() );
    emit sourceChanged();
}

ControlHelper::State GameConsole::getState() {
    return state;
}

qreal GameConsole::getVolume() {
    return volume;
}

void GameConsole::setVolume( qreal volume ) {
    this->volume = volume;
    emit volumeChanged();
}

bool GameConsole::getVsync() {
    return vsync;
}

void GameConsole::setVsync( bool vsync ) {
    this->vsync = vsync;
    emit vsyncChanged();
}
