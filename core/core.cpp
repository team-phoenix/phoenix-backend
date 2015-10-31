#include "core.h"

Core::Core( QObject *parent ) : QObject( parent ),
    pausable( false ),
    playbackSpeed( 1.0 ),
    resettable( false ),
    rewindable( false ),
    source(),
    state( Core::INIT ),
    volume( 1.0 ) {

}

void Core::setState( Core::State state ) {
    qCDebug( phxCore ) << QStringLiteral( "State changed to" ) << state;
}
