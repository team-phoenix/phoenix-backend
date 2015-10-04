#include "gamemanager.h"

GameManager::GameManager( QObject *parent ) : QObject( parent ),

    muted( false ), pausable( false ), playbackRate( 1.0 ), resettable( false ), rewindable( false ),
    source(), state( GameSession::INIT ), volume( 1.0 ), videoOutput( nullptr ),

    gameThread( new QThread() ), gameSession( new GameSession() ) {

    // Notify the QML engine of all the initial property values set in this constructor
    notifyAllProperties();

    // Set up the thread and move gameSession to it
    gameThread->setObjectName( "Game thread" );
    gameSession->moveToThread( gameThread );

    // Catch the user exit signal and clean up
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [ = ]() {

        qCDebug( phxController ) << "===========QCoreApplication::aboutToQuit()===========";

        // INIT/LOADING: Doesn't matter, data is just being read
        // PLAYING/PAUSED/ERROR: Call stop(), should write save data normally then go STOPPED
        // UNLOADING: Critical! Must wait for the state change to STOPPED or writing save data may be interrupted!
        // End the game session
        stop();

        // Stop the game thread
        gameThread->exit();
        gameThread->wait();
        gameThread->deleteLater();

    } );

    // Make sure that the session gets deleted once its thread finishes processing all events
    connect( gameThread, &QThread::finished, gameSession, &GameSession::deleteLater );

    // Launch the thread
    gameThread->start();

    setState( GameSession::STOPPED );
}

void GameManager::load() {
    // check if in "stopped" state, error if not

    // check source for "type" key, error if not found

    setState( GameSession::LOADING );

    // switch( "type" value )
    //     case "libretro":
    //         check for "core", "game", error if not found
    //         check the two exist and are readable by Phoenix, error if not able
    //         emit signalLoadLibretro( core, game );
    //     default:
    //         error, invalid value for "type"

}

void GameManager::play() {
    // check if in "paused" state, error if not

    // send signal to session to resume the timer

    setState( GameSession::PLAYING );
}

void GameManager::pause() {
    // check if in "playing" state, error if not

    // check if pausable, error if not
    if( !pausable ) {
        // error: attempted to pause when not pausable
    }

    // send signal to session to stop the timer

    // Save current playback rate (for when it's resumed) and set the new one
    savedPlaybackRate = playbackRate;
    playbackRate = 0.0;
    emit signalPlaybackRateChanged( playbackRate );

    setState( GameSession::PAUSED );
}

void GameManager::stop() {
    // check if in "playing", "paused" or "error" state, error if not

    // if "playing" or "paused", tell session to end session gracefully
    // otherwise, tell session to end session not so gracefully

    setState( GameSession::UNLOADING );
}

void GameManager::reset() {
    // check if in "playing" or "paused" state, error if not

    // check if we're able to reset or not, error if we can't
    if( !resettable ) {
        // Set error state
        setState( GameSession::ERRORED );
    }

    // send signal to session to reset emulation
}

void GameManager::slotError( QString error ) {
    setState( GameSession::ERRORED );
}

void GameManager::slotLoadFinished() {
    if( pausable ) {
        setState( GameSession::PAUSED );
    } else {
        setState( GameSession::PLAYING );
    }
}

void GameManager::slotUnloadFinished() {
    setState( GameSession::STOPPED );
}

bool GameManager::getPausable() {
    return pausable;
}

bool GameManager::getResettable() {
    return resettable;
}

bool GameManager::getRewindable() {
    return rewindable;
}

GameSession::State GameManager::getState() {
    return state;
}

void GameManager::notifyAllProperties() {
    emit signalErrorChanged( error );
    emit signalMutedChanged( muted );
    emit signalPausableChanged( pausable );
    emit signalPlaybackRateChanged( playbackRate );
    emit signalResettableChanged( resettable );
    emit signalRewindableChanged( rewindable );
    emit signalSourceChanged( source );
    emit signalStateChanged( state );
    emit signalVideoOutputChanged( videoOutput );
    emit signalVolumeChanged( volume );
}

void GameManager::setError( GameSession::Error newError ) {
    error = newError;
    emit signalErrorChanged( error );
}

void GameManager::setState( GameSession::State newState ) {
    state = newState;
    emit signalStateChanged( state );
}
