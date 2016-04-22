#include "core.h"
#include "controlhelper.h"
#include "logging.h"


Core::Core( QObject *parent ) : QObject( parent )
{

}

// Slots

void Core::setPlaybackSpeed( qreal playbackSpeed ) {
    this->playbackSpeed = playbackSpeed;
    emit playbackSpeedChanged( playbackSpeed );
}

void Core::setVolume( qreal volume ) {
    this->volume = volume;
    emit volumeChanged( volume );
}

//void Core::load() {
//    setState( Control::LOADING );
//    setState( Control::PAUSED );
//}

//void Core::play() {
//    setState( Control::PLAYING );
//}

//void Core::pause() {
//    setState( Control::PAUSED );
//}

//void Core::reset() {
//}

//void Core::stop() {
//    setState( Control::UNLOADING );
//    setState( Control::STOPPED );
//}

// Protected

void Core::allPropertiesChanged() {
    emit pausableChanged( pausable );
    emit playbackSpeedChanged( playbackSpeed );
    emit resettableChanged( resettable );
    emit rewindableChanged( rewindable );
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

