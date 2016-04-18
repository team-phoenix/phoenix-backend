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
    gameThread( new QThread() ),
    pausable( false ),
    playbackSpeed( 1.0 ),
    resettable( false ),
    rewindable( false ),
    state( ControlHelper::STOPPED ),
    volume( 1.0 ),
    vsync( false ) {

    // Tell the QML engine the initial values of all properties
    notifyAllProperties();

    // Set up GameConsole
    m_gameConsole->setObjectName( "GameConsole" );
    m_gameConsole->moveToThread( gameThread );
    connect( gameThread, &QThread::finished,  m_gameConsole, &QObject::deleteLater );
    connect( this, &GameConsoleProxy::shutdown,  m_gameConsole, &GameConsole::shutdown );

    connectGameConsoleProxy();

    gameThread->setObjectName( "Game thread" );
    gameThread->start( QThread::HighestPriority );

    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ = ]() {
        qDebug() << "";
        qCInfo( phxControlProxy ) << ">>>>>>>> User requested app to close, shutting down (waiting up to 30 seconds)...";
        qDebug() << "";

        // Shut down gameConsole (calls gameThread->exit() too)
        emit shutdown();

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
    emit loadForwarder();
}

void GameConsoleProxy::play() {
    emit playForwarder();
}

void GameConsoleProxy::pause() {
    emit pauseForwarder();
}

void GameConsoleProxy::stop() {
    emit stopForwarder();
}

void GameConsoleProxy::reset() {
    emit resetForwarder();
}

void GameConsoleProxy::componentComplete() {

    auto args = QCoreApplication::arguments();
    if ( args.size() == 4 ) {
        if ( args.at( 1 ) == QStringLiteral( "-c" ) ) {
            auto core = args.at( 2 );
            auto game = args.at( 3 );

            if ( QFile::exists( core ) && QFile::exists( game ) ) {
                setCoreSrc( core  );
                setGameSrc(  game );
                load();
            }
        }

    } else {
        if ( !source.isEmpty() ) {
            load();
        }
    }
}

// Private slots, cannot be called from QML. Use the respective properties instead

void GameConsoleProxy::setVideoOutput( VideoOutput *videoOutput ) {
    emit videoOutputChangedProxy( videoOutput );
}

void GameConsoleProxy::setPlaybackSpeed( qreal playbackSpeed ) {
    emit playbackSpeedChangedProxy( playbackSpeed );
}

void GameConsoleProxy::setSource( QVariantMap sourceQVariantMap ) {
    // Convert to a QStringMap
    QStringMap sourceQStringMap;

    for( QVariantMap::const_iterator iter = sourceQVariantMap.begin(); iter != sourceQVariantMap.end(); ++iter ) {
        sourceQStringMap[ iter.key() ] = iter.value().toString();
    }

    emit sourceChangedProxy( sourceQStringMap );
}

void GameConsoleProxy::setVolume( qreal volume ) {
    emit volumeChangedProxy( volume );
}

void GameConsoleProxy::setVsync( bool vsync ) {
    emit vsyncChangedProxy( vsync );
}

void GameConsoleProxy::setVideoOutputProxy( VideoOutput *videoOutput ) {
    this->videoOutput = videoOutput;
    emit videoOutputChanged( videoOutput );
}

void GameConsoleProxy::setPausableProxy( bool pausable ) {
    this->pausable = pausable;
    emit pausableChanged( pausable );
}

void GameConsoleProxy::setPlaybackSpeedProxy( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged( playbackSpeed );
}

void GameConsoleProxy::setResettableProxy( bool resettable ) {
    this->resettable = resettable;
    emit resettableChanged( resettable );
}

void GameConsoleProxy::setRewindableProxy( bool rewindable ) {
    this->rewindable = rewindable;
    emit rewindableChanged( rewindable );
}

void GameConsoleProxy::setSourceProxy( QStringMap sourceQStringMap ) {

    // qCDebug( phxControlProxy ) << "Core source change to" << sourceQStringMap << "acknowedged";

    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = sourceQStringMap.begin(); iter != sourceQStringMap.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    this->source = sourceQStringMap;
    emit sourceChanged( sourceQVariantMap );
}

void GameConsoleProxy::setStateProxy( Control::State state ) {
    // qCDebug( phxControlProxy ) << Q_FUNC_INFO << "State change to" << ( ControlHelper::State )state << "acknowledged";
    this->state = ( ControlHelper::State )state;
    emit stateChanged( ( ControlHelper::State )state );
}

void GameConsoleProxy::setVolumeProxy( qreal volume ) {
    this->volume = volume;
    emit volumeChanged( volume );
}

void GameConsoleProxy::setVsyncProxy( bool vsync ) {
    this->vsync = vsync;
    emit vsyncChanged( vsync );
}

void GameConsoleProxy::connectGameConsoleProxy() {

    // Step 2 (our proxy to GameConsole's setter)
    connect( this, &GameConsoleProxy::videoOutputChangedProxy, m_gameConsole, &GameConsole::setVideoOutput );
    connect( this, &GameConsoleProxy::playbackSpeedChangedProxy, m_gameConsole, &GameConsole::setPlaybackSpeed );
    connect( this, &GameConsoleProxy::sourceChangedProxy, m_gameConsole, &GameConsole::setSource );
    connect( this, &GameConsoleProxy::volumeChangedProxy, m_gameConsole, &GameConsole::setVolume );
    connect( this, &GameConsoleProxy::vsyncChangedProxy, m_gameConsole, &GameConsole::setVsync );

    // Step 3 (GameConsole change notifier to our proxy)
    connect( m_gameConsole, &GameConsole::videoOutputChanged, this, &GameConsoleProxy::setVideoOutputProxy );
    connect( m_gameConsole, &GameConsole::pausableChanged, this, &GameConsoleProxy::setPausableProxy );
    connect( m_gameConsole, &GameConsole::playbackSpeedChanged, this, &GameConsoleProxy::setPlaybackSpeedProxy );
    connect( m_gameConsole, &GameConsole::resettableChanged, this, &GameConsoleProxy::setResettableProxy );
    connect( m_gameConsole, &GameConsole::rewindableChanged, this, &GameConsoleProxy::setRewindableProxy );
    connect( m_gameConsole, &GameConsole::sourceChanged, this, &GameConsoleProxy::setSourceProxy );
    connect( m_gameConsole, &GameConsole::volumeChanged, this, &GameConsoleProxy::setVolumeProxy );
    connect( m_gameConsole, &GameConsole::stateChanged, this, &GameConsoleProxy::setStateProxy );
    connect( m_gameConsole, &GameConsole::vsyncChanged, this, &GameConsoleProxy::setVsyncProxy );

    // Connect the methods, too
    connect( this, &GameConsoleProxy::loadForwarder,m_gameConsole, &GameConsole::load );
    connect( this, &GameConsoleProxy::playForwarder, m_gameConsole, &GameConsole::play );
    connect( this, &GameConsoleProxy::pauseForwarder, m_gameConsole, &GameConsole::pause );
    connect( this, &GameConsoleProxy::stopForwarder, m_gameConsole, &GameConsole::stop );
    connect( this, &GameConsoleProxy::resetForwarder, m_gameConsole, &GameConsole::reset );

}

VideoOutput *GameConsoleProxy::getVideoOutput() const {
    return videoOutput;
}


void GameConsoleProxy::notifyAllProperties() {
    emit videoOutputChanged( videoOutput );
    emit pausableChanged( pausable );
    emit playbackSpeedChanged( playbackSpeed );
    emit resettableChanged( resettable );
    emit rewindableChanged( rewindable );
    //emit sourceChanged( getSource() );
    emit stateChanged( state );
    emit volumeChanged( volume );
    emit vsyncChanged( vsync );
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

QVariantMap GameConsoleProxy::getSource() const {
    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = source.begin(); iter != source.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    return sourceQVariantMap;
}

ControlHelper::State GameConsoleProxy::getState() const {
    return ( ControlHelper::State )state;
}

qreal GameConsoleProxy::getVolume() const {
    return volume;
}

bool GameConsoleProxy::getVsync() const {
    return vsync;
}

