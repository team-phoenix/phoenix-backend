#include "gamesession.h"

GameSession::GameSession( QObject *parent ) : QObject( parent ),

    error( GameSession::NOERR ), muted( false ), pausable( false ), playbackRate( 1.0 ), resettable( false ),
    rewindable( false ), source(), state( GameSession::INIT ), videoOutput( nullptr ), volume( 1.0 ),

    libretroCore() {

    // Connect core signals and slots
    connect( this, &GameSession::loadLibretroCore, &libretroCore, &LibretroCore::slotLoadCore );
    connect( this, &GameSession::loadLibretroGame, &libretroCore, &LibretroCore::slotLoadGame );

    emit signalInitFinished();

}

void GameSession::loadLibretro( QString core, QString game ) {

    emit loadLibretroCore( core );
    emit loadLibretroGame( game );

    emit signalLoadFinished();
}

