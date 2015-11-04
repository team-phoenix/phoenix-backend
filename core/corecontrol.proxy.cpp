#include "corecontrol.h"

// Safe to call from QML

void CoreControl::load() {
    emit loadProxy();
}

void CoreControl::play() {
    emit playProxy();
}

void CoreControl::pause() {
    emit pauseProxy();
}

void CoreControl::stop() {
    emit stopProxy();
}

void CoreControl::reset() {
    emit resetProxy();
}

// Private, cannot be called from QML. Use the respective properties instead

void CoreControl::setPausableProxy( bool pausable ) {
    this->pausable = pausable;
    emit pausableChanged( pausable );
}

void CoreControl::setPlaybackSpeed( qreal playbackSpeed ) {
    emit playbackSpeedChangedProxy( playbackSpeed );
}

void CoreControl::setSource( QVariantMap sourceQVariantMap ) {

    // Convert to a QStringMap
    QStringMap sourceQStringMap;

    for( QVariantMap::const_iterator iter = sourceQVariantMap.begin(); iter != sourceQVariantMap.end(); ++iter ) {
        sourceQStringMap[ iter.key() ] = iter.value().toString();
    }

    // Determine Core type, instantiate appropiate type
    if( sourceQStringMap[ QStringLiteral( "type" ) ] == QStringLiteral( "libretro" ) ) {
        initLibretro();
        qCDebug( phxController ) << "LibretroCore fully initalized and connected";
    } else {
        qCCritical( phxController ).nospace() << QStringLiteral( "Unknown type " )
                                              << sourceQStringMap[ "type" ] << QStringLiteral( " passed to load()!" );
    }

    emit sourceChangedProxy( sourceQStringMap );

}

void CoreControl::setVolume( qreal volume ) {
    this->volume = volume;
    emit volumeChangedProxy( volume );
}

void CoreControl::setPlaybackSpeedProxy( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged( playbackSpeed );
}

void CoreControl::setResettableProxy( bool resettable ) {
    this->resettable = resettable;
    emit resettableChanged( resettable );
}

void CoreControl::setRewindableProxy( bool rewindable ) {
    this->rewindable = rewindable;
    emit rewindableChanged( rewindable );
}

void CoreControl::setSourceProxy( QStringMap sourceQStringMap ) {

    // qCDebug( phxController ) << "Core source change to" << sourceQStringMap << "acknowedged";

    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = sourceQStringMap.begin(); iter != sourceQStringMap.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    this->source = sourceQStringMap;
    emit sourceChanged( sourceQVariantMap );
}

void CoreControl::setStateProxy( Core::State state ) {
    // qCDebug( phxController ) << Q_FUNC_INFO << "State change to" << state << "acknowledged";
    this->state = state;
    emit stateChanged( state );
}

void CoreControl::setVolumeProxy( qreal volume ) {
    this->volume = volume;
    emit volumeChanged( volume );
}

void CoreControl::connectCoreProxy() {

    // Step 2 (our proxy to Core's setter)
    connect( this, &CoreControl::playbackSpeedChangedProxy, core, &Core::setPlaybackSpeed );
    connect( this, &CoreControl::sourceChangedProxy, core, &Core::setSource );
    connect( this, &CoreControl::volumeChangedProxy, core, &Core::setVolume );

    // Step 3 (Core change notifier to our proxy)
    connect( core, &Core::pausableChanged, this, &CoreControl::setPausableProxy );
    connect( core, &Core::playbackSpeedChanged, this, &CoreControl::setPlaybackSpeedProxy );
    connect( core, &Core::resettableChanged, this, &CoreControl::setResettableProxy );
    connect( core, &Core::rewindableChanged, this, &CoreControl::setRewindableProxy );
    connect( core, &Core::sourceChanged, this, &CoreControl::setSourceProxy );
    connect( core, &Core::volumeChanged, this, &CoreControl::setVolumeProxy );
    connect( core, &Core::stateChanged, this, &CoreControl::setStateProxy );

    // Connect the methods, too
    connect( this, &CoreControl::loadProxy, core, &Core::load );
    connect( this, &CoreControl::playProxy, core, &Core::play );
    connect( this, &CoreControl::pauseProxy, core, &Core::pause );
    connect( this, &CoreControl::stopProxy, core, &Core::stop );
    connect( this, &CoreControl::resetProxy, core, &Core::reset );

}

void CoreControl::notifyAllProperties() {
    emit pausableChanged( pausable );
    emit playbackSpeedChanged( playbackSpeed );
    emit resettableChanged( resettable );
    emit rewindableChanged( rewindable );
    emit sourceChanged( getSource() );
    emit stateChanged( state );
    emit volumeChanged( volume );
}

bool CoreControl::getPausable() const {
    return pausable;
}

qreal CoreControl::getPlaybackSpeed() const {
    return playbackSpeed;
}

bool CoreControl::getResettable() const {
    return resettable;
}

bool CoreControl::getRewindable() const {
    return rewindable;
}

QVariantMap CoreControl::getSource() const {
    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = source.begin(); iter != source.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    return sourceQVariantMap;
}

Core::State CoreControl::getState() const {
    return state;
}

qreal CoreControl::getVolume() const {
    return volume;
}

