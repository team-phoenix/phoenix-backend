#include "core.h"

Core::Core( QObject *parent ) : QObject( parent ),
    pausable( false ),
    playbackSpeed( 1.0 ),
    resettable( false ),
    rewindable( false ),
    source(),
    state( Core::INIT ),
    volume( 1.0 ) {
    qCDebug( phxCore ) << Q_FUNC_INFO;
}

Core::~Core() {

}

// Slots

void Core::setPlaybackSpeed( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged( playbackSpeed );
}

void Core::setSource( QStringMap source ) {
    this->source = source;
    emit sourceChanged( source );
}

void Core::setVolume( qreal volume ) {
    this->volume = volume;
    emit volumeChanged( volume );
}

void Core::load() {
    setState( LOADING );
    setState( PAUSED );
}

void Core::play() {
    setState( PLAYING );
}

void Core::pause() {
    setState( PAUSED );
}

void Core::reset() {
}

void Core::stop() {
    setState( UNLOADING );
    setState( STOPPED );
}

// Protected

void Core::allPropertiesChanged() {
    emit pausableChanged( pausable );
    emit playbackSpeedChanged( playbackSpeed );
    emit resettableChanged( resettable );
    emit rewindableChanged( rewindable );
    emit sourceChanged( source );
    emit stateChanged( state );
    emit volumeChanged( volume );
}

void Core::setPausable( bool pausable ) {
    this->pausable = pausable;
    emit pausableChanged( pausable );
}

void Core::setResettable( bool resettable ) {
    this->resettable = resettable;
    emit resettableChanged( resettable );
}

void Core::setRewindable( bool rewindable ) {
    this->rewindable = rewindable;
    emit rewindableChanged( rewindable );
}

void Core::setState( Core::State state ) {
    this->state = state;
    emit stateChanged( state );
    qCDebug( phxCore ) << Q_FUNC_INFO << "State changed to" << state;
}
