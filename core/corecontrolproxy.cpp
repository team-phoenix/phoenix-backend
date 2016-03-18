#include "corecontrolproxy.h"

#include <type_traits>
#include <QQmlApplicationEngine>

CoreControlProxy::CoreControlProxy( QObject *parent ) : QObject( parent ),
    coreControl( new CoreControl() ),
    gameThread( new QThread() ),
    pausable( false ),
    playbackSpeed( 1.0 ),
    resettable( false ),
    rewindable( false ),
    source(),
    state( ControlHelper::STOPPED ),
    volume( 1.0 ),
    vsync( false ) {

    // Tell the QML engine the initial values of all properties
    notifyAllProperties();

    // Set up CoreControl
    coreControl->setObjectName( "CoreControl" );
    coreControl->moveToThread( gameThread );
    connect( gameThread, &QThread::finished, coreControl, &QObject::deleteLater );
    connect( this, &CoreControlProxy::shutdown, coreControl, &CoreControl::shutdown );

    connectCoreControlProxy();

    gameThread->setObjectName( "Game thread" );
    gameThread->start( QThread::HighestPriority );

    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ = ]() {
        qDebug() << "";
        qCInfo( phxControlProxy ) << ">>>>>>>> User requested app to close, shutting down (waiting up to 30 seconds)...";
        qDebug() << "";

        // Shut down coreControl (calls gameThread->exit() too)
        emit shutdown();

        // Shut down thread, block until it finishes
        gameThread->wait( 30 * 1000 );
        gameThread->deleteLater();

        qDebug() << "";
        qCInfo( phxControlProxy ) << ">>>>>>>> Fully unloaded, quitting!";
        qDebug() << "";
    } );
}

CoreControlProxy::~CoreControlProxy() {

}

// Safe to call from QML

void CoreControlProxy::load() {
    emit loadForwarder();
}

void CoreControlProxy::play() {
    emit playForwarder();
}

void CoreControlProxy::pause() {
    emit pauseForwarder();
}

void CoreControlProxy::stop() {
    emit stopForwarder();
}

void CoreControlProxy::reset() {
    emit resetForwarder();
}

// Private slots, cannot be called from QML. Use the respective properties instead

void CoreControlProxy::setVideoOutput( QObject *videoOutput ) {
    VideoOutput *obj = dynamic_cast<VideoOutput *>( videoOutput );
    Q_ASSERT_X( obj, Q_FUNC_INFO, "QObject * is not of type VideoOutput *" );
    emit videoOutputChangedProxy( obj );
}

void CoreControlProxy::setInputManager( QObject *inputManager ) {
    InputManager *obj = dynamic_cast<InputManager *>( inputManager );
    Q_ASSERT_X( obj, Q_FUNC_INFO, "QObject * is not of type InputManager *" );
    emit inputManagerChangedProxy( obj );
}

void CoreControlProxy::setPlaybackSpeed( qreal playbackSpeed ) {
    emit playbackSpeedChangedProxy( playbackSpeed );
}

void CoreControlProxy::setSource( QVariantMap sourceQVariantMap ) {
    // Convert to a QStringMap
    QStringMap sourceQStringMap;

    for( QVariantMap::const_iterator iter = sourceQVariantMap.begin(); iter != sourceQVariantMap.end(); ++iter ) {
        sourceQStringMap[ iter.key() ] = iter.value().toString();
    }

    emit sourceChangedProxy( sourceQStringMap );
}

void CoreControlProxy::setVolume( qreal volume ) {
    emit volumeChangedProxy( volume );
}

void CoreControlProxy::setVsync( bool vsync ) {
    emit vsyncChangedProxy( vsync );
}

void CoreControlProxy::setVideoOutputProxy( VideoOutput *videoOutput ) {
    this->videoOutput = videoOutput;
    emit videoOutputChanged( videoOutput );
}

void CoreControlProxy::setInputManagerProxy( InputManager *inputManager ) {
    this->inputManager = inputManager;
    emit inputManagerChanged( inputManager );
}

void CoreControlProxy::setPausableProxy( bool pausable ) {
    this->pausable = pausable;
    emit pausableChanged( pausable );
}

void CoreControlProxy::setPlaybackSpeedProxy( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged( playbackSpeed );
}

void CoreControlProxy::setResettableProxy( bool resettable ) {
    this->resettable = resettable;
    emit resettableChanged( resettable );
}

void CoreControlProxy::setRewindableProxy( bool rewindable ) {
    this->rewindable = rewindable;
    emit rewindableChanged( rewindable );
}

void CoreControlProxy::setSourceProxy( QStringMap sourceQStringMap ) {

    // qCDebug( phxControlProxy ) << "Core source change to" << sourceQStringMap << "acknowedged";

    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = sourceQStringMap.begin(); iter != sourceQStringMap.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    this->source = sourceQStringMap;
    emit sourceChanged( sourceQVariantMap );
}

void CoreControlProxy::setStateProxy( Control::State state ) {
    // qCDebug( phxControlProxy ) << Q_FUNC_INFO << "State change to" << ( ControlHelper::State )state << "acknowledged";
    this->state = ( ControlHelper::State )state;
    emit stateChanged( ( ControlHelper::State )state );
}

void CoreControlProxy::setVolumeProxy( qreal volume ) {
    this->volume = volume;
    emit volumeChanged( volume );
}

void CoreControlProxy::setVsyncProxy( bool vsync ) {
    this->vsync = vsync;
    emit vsyncChanged( vsync );
}

void CoreControlProxy::connectCoreControlProxy() {

    // Step 2 (our proxy to CoreControl's setter)
    connect( this, &CoreControlProxy::videoOutputChangedProxy, coreControl, &CoreControl::setVideoOutput );
    connect( this, &CoreControlProxy::inputManagerChangedProxy, coreControl, &CoreControl::setInputManager );
    connect( this, &CoreControlProxy::playbackSpeedChangedProxy, coreControl, &CoreControl::setPlaybackSpeed );
    connect( this, &CoreControlProxy::sourceChangedProxy, coreControl, &CoreControl::setSource );
    connect( this, &CoreControlProxy::volumeChangedProxy, coreControl, &CoreControl::setVolume );
    connect( this, &CoreControlProxy::vsyncChangedProxy, coreControl, &CoreControl::setVsync );

    // Step 3 (CoreControl change notifier to our proxy)
    connect( coreControl, &CoreControl::videoOutputChanged, this, &CoreControlProxy::setVideoOutputProxy );
    connect( coreControl, &CoreControl::inputManagerChanged, this, &CoreControlProxy::setInputManagerProxy );
    connect( coreControl, &CoreControl::pausableChanged, this, &CoreControlProxy::setPausableProxy );
    connect( coreControl, &CoreControl::playbackSpeedChanged, this, &CoreControlProxy::setPlaybackSpeedProxy );
    connect( coreControl, &CoreControl::resettableChanged, this, &CoreControlProxy::setResettableProxy );
    connect( coreControl, &CoreControl::rewindableChanged, this, &CoreControlProxy::setRewindableProxy );
    connect( coreControl, &CoreControl::sourceChanged, this, &CoreControlProxy::setSourceProxy );
    connect( coreControl, &CoreControl::volumeChanged, this, &CoreControlProxy::setVolumeProxy );
    connect( coreControl, &CoreControl::stateChanged, this, &CoreControlProxy::setStateProxy );
    connect( coreControl, &CoreControl::vsyncChanged, this, &CoreControlProxy::setVsyncProxy );

    // Connect the methods, too
    connect( this, &CoreControlProxy::loadForwarder, coreControl, &CoreControl::load );
    connect( this, &CoreControlProxy::playForwarder, coreControl, &CoreControl::play );
    connect( this, &CoreControlProxy::pauseForwarder, coreControl, &CoreControl::pause );
    connect( this, &CoreControlProxy::stopForwarder, coreControl, &CoreControl::stop );
    connect( this, &CoreControlProxy::resetForwarder, coreControl, &CoreControl::reset );

}

VideoOutput *CoreControlProxy::getVideoOutput() const {
    return videoOutput;
}

InputManager *CoreControlProxy::getInputManager() const {
    return inputManager;
}

void CoreControlProxy::notifyAllProperties() {
    emit videoOutputChanged( videoOutput );
    emit inputManagerChanged( inputManager );
    emit pausableChanged( pausable );
    emit playbackSpeedChanged( playbackSpeed );
    emit resettableChanged( resettable );
    emit rewindableChanged( rewindable );
    emit sourceChanged( getSource() );
    emit stateChanged( state );
    emit volumeChanged( volume );
    emit vsyncChanged( vsync );
}

bool CoreControlProxy::getPausable() const {
    return pausable;
}

qreal CoreControlProxy::getPlaybackSpeed() const {
    return playbackSpeed;
}

bool CoreControlProxy::getResettable() const {
    return resettable;
}

bool CoreControlProxy::getRewindable() const {
    return rewindable;
}

QVariantMap CoreControlProxy::getSource() const {
    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = source.begin(); iter != source.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    return sourceQVariantMap;
}

ControlHelper::State CoreControlProxy::getState() const {
    return ( ControlHelper::State )state;
}

qreal CoreControlProxy::getVolume() const {
    return volume;
}

bool CoreControlProxy::getVsync() const {
    return vsync;
}

