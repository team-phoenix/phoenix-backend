#include "gameconsole.h"

#include "logging.h"

GameConsole::GameConsole( Node *parent ) : Node( parent ),
    gameThread( new QThread() ),
    audioOutput( new AudioOutput() ),
    gamepadManager( new GamepadManager() ),
    microTimer( new MicroTimer() ),
    remapper( new Remapper() ) {
    // Move all our stuff to the game thread
    audioOutput->moveToThread( gameThread );
    gamepadManager->moveToThread( gameThread );
    microTimer->moveToThread( gameThread );
    remapper->moveToThread( gameThread );

    // Connect global pipeline
    // TODO

    // Begin timer so it may poll for input
    emit controlOut( Command::HeartbeatRate, 60.0, QDateTime::currentMSecsSinceEpoch() );

    // Handle app quits
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ = ]() {
        qDebug() << "";
        qCInfo( phxControl ) << ">>>>>>>> User requested app to close, shutting down (waiting up to 30 seconds)...";
        qDebug() << "";

        // Tell the pipeline to stop
        controlOut( Command::Stop, QVariant(), QDateTime::currentMSecsSinceEpoch() );

        // Wait up to 30 seconds to let the pipeline finish its events
        gameThread->wait( 30 * 1000 );
        gameThread->deleteLater();

        qDebug() << "";
        qCInfo( phxControl ) << ">>>>>>>> Fully unloaded, quitting!";
        qDebug() << "";
    } );
}

GameConsole::~GameConsole() {
    delete audioOutput;
    delete gamepadManager;
    delete microTimer;
    delete remapper;

    delete gameThread;
}

// Public slots

void GameConsole::load() {
    if( source[ "type" ] == QStringLiteral( "libretro" ) ) {
        loadLibretro();
    } else if( source[ "type" ].toString().isEmpty() ) {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Source was not set!" );
    } else {
        qCCritical( phxControl ).nospace() << QStringLiteral( "Unknown type " )
                                           << source[ "type" ] << QStringLiteral( " passed to load()!" );
    }
}

void GameConsole::play() {

}

void GameConsole::pause() {

}

void GameConsole::stop() {

}

void GameConsole::reset() {

}

// Private

void GameConsole::loadLibretro() {
    // Ensure that the properties were set in QML
    Q_ASSERT_X( metaOutput, "libretro load", "metaOutput was not set!" );
    Q_ASSERT_X( videoOutput, "libretro load", "videoOutput was not set!" );

    // Connect the nodes together
    // TODO
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
