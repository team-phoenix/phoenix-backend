#include "gameconsoleproxy.h"
#include "gameconsole.h"
#include "logging.h"

#include <type_traits>

#include <QThread>
#include <QCoreApplication>
#include <QQmlApplicationEngine>

using namespace Input;

GameConsoleProxy::GameConsoleProxy( QObject *parent ) : QObject( parent ),
    gameConsole( new GameConsole ),
    gameThread( new QThread ) {

    // Set up GameConsole
    gameConsole->setObjectName( "GameConsole" );
    gameConsole->moveToThread( gameThread );
    connect( gameThread, &QThread::finished,  gameConsole, &QObject::deleteLater );

    gameThread->setObjectName( "Game thread" );
    gameThread->start( QThread::HighestPriority );

    connect( gameConsole, &GameConsole::gamepadAdded, this, &GameConsoleProxy::gamepadAdded );
    connect( gameConsole, &GameConsole::gamepadRemoved, this, &GameConsoleProxy::gamepadRemoved );

    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ = ]() {
        qDebug() << "";
        qCInfo( phxControlProxy ) << ">>>>>>>> User requested app to close, shutting down (waiting up to 30 seconds)...";
        qDebug() << "";

        // Shut down gameConsole (calls gameThread->exit() too)

        QMetaObject::invokeMethod( gameConsole, "shutdown" );

        // Shut down thread, block until it finishes
        gameThread->wait( 30 * 1000 );
        gameThread->deleteLater();

        qDebug() << "";
        qCInfo( phxControlProxy ) << ">>>>>>>> Fully unloaded, quitting!";
        qDebug() << "";
    } );


}


// Safe to call from QML

void GameConsoleProxy::load() {
    setPlaybackState( PlaybackState::Loading );
}

void GameConsoleProxy::play() {
    setPlaybackState( PlaybackState::Playing );
}

void GameConsoleProxy::pause() {
    setPlaybackState( PlaybackState::Paused );
}

void GameConsoleProxy::stop() {
    setPlaybackState( PlaybackState::Stopped );
}

void GameConsoleProxy::reset() {
    setPlaybackState( PlaybackState::Resetting );
}

void GameConsoleProxy::classBegin() {
    // Tell QML that our signals have changed.
    emit sourceChanged();
    emit videoOutputChanged();
    emit pausableChanged();
    emit playbackSpeedChanged();
    emit resettableChanged();
    emit rewindableChanged();
    emit sourceChanged();
    emit volumeChanged();
    emit vsyncChanged();
}

QVariantMap GameConsoleProxy::getSource() const {
    return source;
}

void GameConsoleProxy::setSource( QVariantMap source ) {
    QMetaObject::invokeMethod( gameConsole, "setSource", Q_ARG( QVariantMap, source ) );
    this->source = source;
    emit sourceChanged();
}

void GameConsoleProxy::setVideoOutput( VideoOutput *videoOutput ) {
    QMetaObject::invokeMethod( gameConsole, "setVideoOutput", Q_ARG( VideoOutput *, videoOutput ) );
    this->videoOutput = videoOutput;
    emit videoOutputChanged();
}

void GameConsoleProxy::setPlaybackSpeed( qreal speed ) {
    QMetaObject::invokeMethod( gameConsole, "setPlaybackSpeed", Q_ARG( qreal, speed ) );
    this->playbackSpeed = speed;
    emit playbackSpeedChanged();
}

void GameConsoleProxy::setVolume( qreal volume ) {
    QMetaObject::invokeMethod( gameConsole, "setVolume", Q_ARG( qreal, volume ) );
    this->volume = volume;
    emit volumeChanged();
}

void GameConsoleProxy::setVsync( bool vsync ) {
    QMetaObject::invokeMethod( gameConsole, "setVsync", Q_ARG( bool, vsync ) );
    this->vsync = vsync;
    emit vsyncChanged();
}

void GameConsoleProxy::setRewindable( bool rewindable ) {
    this->rewindable = rewindable;
    emit rewindableChanged();
}

void GameConsoleProxy::setResettable( bool resettable ) {
    this->resettable = resettable;
    emit resettableChanged();
}

void GameConsoleProxy::setPausable( bool pausable ) {
    this->pausable = pausable;
    emit pausableChanged();
}

void GameConsoleProxy::setPlaybackState( GameConsoleProxy::PlaybackState state ) {
    playbackState = state;

    switch( state ) {
        case PlaybackState::Loading:
            QMetaObject::invokeMethod( gameConsole, "load" );
            break;

        case PlaybackState::Unloading:
            //QMetaObject::invokeMethod( m_gameConsole, "load" );
            break;

        case PlaybackState::Stopped:
            QMetaObject::invokeMethod( gameConsole, "stop" );
            break;

        case PlaybackState::Playing:
            QMetaObject::invokeMethod( gameConsole, "play" );
            break;

        case PlaybackState::Paused:
            QMetaObject::invokeMethod( gameConsole, "pause" );
            break;

        case PlaybackState::Resetting:
            QMetaObject::invokeMethod( gameConsole, "reset" );
            break;

        default:
            Q_UNREACHABLE();
            break;
    }

    emit playbackStateChanged();
}

VideoOutput *GameConsoleProxy::getVideoOutput() const {
    return videoOutput;
}

bool GameConsoleProxy::getPausable() const {
    return pausable;
}

qreal GameConsoleProxy::getPlaybackSpeed() const {
    return playbackSpeed;
}

bool GameConsoleProxy::getResettable() const {
    return resettable;
}

bool GameConsoleProxy::getRewindable() const {
    return rewindable;
}

qreal GameConsoleProxy::getVolume() const {
    return volume;
}

bool GameConsoleProxy::getVsync() const {
    return vsync;
}

GameConsoleProxy::PlaybackState GameConsoleProxy::getPlaybackState() const {
    return playbackState;
}

