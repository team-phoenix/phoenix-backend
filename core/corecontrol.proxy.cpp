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

// None of these methods should be called directly from QML
// Use the respective properties instead

void CoreControl::setPlaybackSpeed( qreal playbackSpeed ) {
    emit playbackSpeedChangedProxy( playbackSpeed );
}

void CoreControl::setSource( QVariantMap sourceQVariantMap ) {

    qCDebug( phxController ) << sourceQVariantMap;

    // Convert to a QStringMap
    QStringMap sourceQStringMap;

    for( QVariantMap::const_iterator iter = sourceQVariantMap.begin(); iter != sourceQVariantMap.end(); ++iter ) {
        sourceQStringMap[ iter.key() ] = iter.value().toString();
    }

    qCDebug( phxController ) << sourceQStringMap;

    // Determine Core type, instantiate appropiate type
    if( sourceQStringMap[ QStringLiteral( "type" ) ] == QStringLiteral( "libretro" ) ) {
        loadLibretro();
    } else {
        qCCritical( phxController ).nospace() << QStringLiteral( "Unknown type " )
                                              << sourceQStringMap[ "type" ] << QStringLiteral( " passed to load()!" );
    }

    emit sourceChangedProxy( sourceQStringMap );

}

void CoreControl::setVolume( qreal volume ) {
    emit volumeChangedProxy( volume );
}

void CoreControl::setPlaybackSpeedProxy( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged( playbackSpeed );
}

void CoreControl::setSourceProxy( QStringMap sourceQStringMap ) {

    qCDebug( phxController ) << "Core source change to" << sourceQStringMap << "acknowedged";

    // Convert to a QVariantMap
    QVariantMap sourceQVariantMap;

    for( QStringMap::const_iterator iter = sourceQStringMap.begin(); iter != sourceQStringMap.end(); ++iter ) {
        sourceQVariantMap[ iter.key() ] = iter.value();
    }

    this->source = sourceQStringMap;
    emit sourceChanged( sourceQVariantMap );
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
    connect( core, &Core::playbackSpeedChanged, this, &CoreControl::setPlaybackSpeedProxy );
    connect( core, &Core::sourceChanged, this, &CoreControl::setSourceProxy );
    connect( core, &Core::volumeChanged, this, &CoreControl::setVolumeProxy );

    // Handle the other ones changing
    // These are lambdas instead of slots so as to prevent them from being changed from QML
    connect( core, &Core::pausableChanged, [ = ]( bool pausable ) {
        this->pausable = pausable;
        emit pausableChanged( pausable );
    } );
    connect( core, &Core::resettableChanged, [ = ]( bool resettable ) {
        this->resettable = resettable;
        emit resettableChanged( resettable );
    } );
    connect( core, &Core::rewindableChanged, [ = ]( bool rewindable ) {
        this->rewindable = rewindable;
        emit rewindableChanged( rewindable );
    } );
    connect( core, &Core::stateChanged, [ = ]( Core::State state ) {
        this->state = state;
        emit stateChanged( state );
    } );

    // Connect the methods, too
    connect( this, &CoreControl::loadProxy, core, &Core::load );
    connect( this, &CoreControl::playProxy, core, &Core::play );
    connect( this, &CoreControl::pauseProxy, core, &Core::pause );
    connect( this, &CoreControl::stopProxy, core, &Core::stop );
    connect( this, &CoreControl::resetProxy, core, &Core::reset );

}

// Good luck calling these from QML (don't try anyway)

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

