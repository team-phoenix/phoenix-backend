#include "libretrovariableforwarder.h"

#include <QDebug>

LibretroVariableForwarder::LibretroVariableForwarder(QObject *parent) : Node(parent)
{

}

void LibretroVariableForwarder::commandIn(Node::Command command, QVariant data, qint64 timeStamp) {

    if ( Command::LibretroVariablesEmitted == command ) {
        emit variableFound( data.value<LibretroVariable>() );
    } else if ( Command::SetLibretroVariable == command ) {
        Node::commandIn( command, data, timeStamp );
    }

    // Do not emit commands unspecific to the LibretroVariableModel, or else the
    // commands calls will double because the other nodes are already emitting those other signals.

}
