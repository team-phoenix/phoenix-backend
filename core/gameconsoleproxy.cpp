#include "gameconsoleproxy.h"
#include "gameconsole.h"
#include "logging.h"

#include <type_traits>

#include <QThread>
#include <QCoreApplication>
#include <QQmlApplicationEngine>

using namespace Input;

GameConsoleProxy::GameConsoleProxy( QObject *parent ) : QObject( parent ),
    m_gameConsole( new GameConsole ),
    gameThread( new QThread )
{

    // Set up GameConsole
    m_gameConsole->setObjectName( "GameConsole" );
    m_gameConsole->moveToThread( gameThread );
    connect( gameThread, &QThread::finished,  m_gameConsole, &QObject::deleteLater );

    gameThread->setObjectName( "Game thread" );
    gameThread->start( QThread::HighestPriority );

    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ = ]() {
        qDebug() << "";
        qCInfo( phxControlProxy ) << ">>>>>>>> User requested app to close, shutting down (waiting up to 30 seconds)...";
        qDebug() << "";

        // Shut down gameConsole (calls gameThread->exit() too)

        QMetaObject::invokeMethod( m_gameConsole, "shutdown" );

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

void GameConsoleProxy::componentComplete() {

    QMetaObject::invokeMethod( m_gameConsole, "componentComplete" );

    if ( m_src.contains( QStringLiteral( "--libretro") ) ) {
        const QString core = m_src[ QStringLiteral( "-c" ) ].toString() ;
        const QString game = m_src[ QStringLiteral( "-g" ) ].toString();
        if ( !core.isEmpty()
             && !game.isEmpty()
             && QFile::exists( core )
             && QFile::exists( game ) ) {

            setSrc( QVariantMap{
                        { QStringLiteral( "core" ), core },
                        { QStringLiteral( "game" ), game },
                    } );

            load();
            play();
        }

    }

    else {
        if ( !m_src.isEmpty() ) {
            load();
            play();
        }
    }



}

void GameConsoleProxy::classBegin() {

    // Tell QML that our signals have changed.
    emit srcChanged();
    emit videoOutputChanged();
    emit pausableChanged();
    emit playbackSpeedChanged();
    emit resettableChanged();
    emit rewindableChanged();
    emit sourceChanged();
    emit volumeChanged();
    emit vsyncChanged();
}

QVariantMap GameConsoleProxy::src() const
{
    return m_src;
}

void GameConsoleProxy::setSrc(QVariantMap t_src) {
    if ( t_src != m_src ) {
        m_src = t_src;
        QMetaObject::invokeMethod( m_gameConsole, "setSrc", Q_ARG( QVariantMap, t_src ) );
        emit srcChanged();
    }
}

void GameConsoleProxy::setVideoOutput( VideoOutput *t_output ) {
    QMetaObject::invokeMethod( m_gameConsole, "setVideoOutput", Q_ARG( VideoOutput *, t_output ) );
    m_videoOutput = t_output;
    emit videoOutputChanged();
}

void GameConsoleProxy::setPlaybackSpeed( qreal t_speed ) {
    QMetaObject::invokeMethod( m_gameConsole, "setPlaybackSpeed", Q_ARG( qreal, t_speed ) );
    m_playbackSpeed = t_speed;
    emit playbackSpeedChanged();
}

void GameConsoleProxy::setVolume( qreal t_volume ) {
    QMetaObject::invokeMethod( m_gameConsole, "setVolume", Q_ARG( qreal, t_volume ) );
    m_volume = t_volume;
    emit volumeChanged();
}

void GameConsoleProxy::setVsync( bool t_vsync ) {
    QMetaObject::invokeMethod( m_gameConsole, "setVsync", Q_ARG( bool, t_vsync ) );
    m_vsync = t_vsync;
    emit vsyncChanged();
}

void GameConsoleProxy::setRewindable(bool t_rewindable) {
    m_rewindable = t_rewindable;
    emit rewindableChanged();
}

void GameConsoleProxy::setResettable(bool t_resettable) {
    m_resettable = t_resettable;
    emit resettableChanged();
}

void GameConsoleProxy::setPausable(bool t_pausable) {
    m_pausable = t_pausable;
    emit pausableChanged();
}

void GameConsoleProxy::setPlaybackState(GameConsoleProxy::PlaybackState t_state) {
    m_playbackState = t_state;
    switch( t_state ) {
        case PlaybackState::Loading:
            QMetaObject::invokeMethod( m_gameConsole, "load" );
            break;
        case PlaybackState::Unloading:
            //QMetaObject::invokeMethod( m_gameConsole, "load" );
            break;
        case PlaybackState::Stopped:
            QMetaObject::invokeMethod( m_gameConsole, "stop" );
            break;
        case PlaybackState::Playing:
            QMetaObject::invokeMethod( m_gameConsole, "play" );
            break;
        case PlaybackState::Paused:
            QMetaObject::invokeMethod( m_gameConsole, "pause" );
            break;
        case PlaybackState::Resetting:
            QMetaObject::invokeMethod( m_gameConsole, "reset" );
            break;
        default:
            Q_UNREACHABLE();
            break;
    }

    emit playbackStateChanged();
}

VideoOutput *GameConsoleProxy::videoOutput() const {
    return m_videoOutput;
}

bool GameConsoleProxy::pausable() const {
    return m_pausable;
}

qreal GameConsoleProxy::playbackSpeed() const {
    return m_playbackSpeed;
}

bool GameConsoleProxy::resettable() const {
    return m_resettable;
}

bool GameConsoleProxy::rewindable() const {
    return m_rewindable;
}

qreal GameConsoleProxy::volume() const {
    return m_volume;
}

bool GameConsoleProxy::vsync() const {
    return m_vsync;
}

GameConsoleProxy::PlaybackState GameConsoleProxy::playbackState() const {
    return m_playbackState;
}

