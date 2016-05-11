#include "libretrovariableforwarder.h"

#include <QDebug>

LibretroVariableForwarder::LibretroVariableForwarder( QObject *parent ) : Node( parent ) {

}

void LibretroVariableForwarder::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    switch( command ) {
        case Command::LibretroVariablesEmitted:
            emit variableFound( data.value<LibretroVariable>() );
            break;

        case Command::SetLibretroVariable:
            emit commandOut( command, data, timeStamp );
            break;

        case Command::Stop:
            emit clearVariables();
            break;

        default:
            break;
    }

    // Do not emit commands unspecific to the LibretroVariableModel, or else the
    // commands calls will double because the other nodes are already emitting those other signals.

}
