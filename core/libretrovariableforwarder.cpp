#include "libretrovariableforwarder.h"
#include "libretrovariablemodel.h"

#include <QDebug>

LibretroVariableForwarder::LibretroVariableForwarder( QObject *parent ) : Node( parent ) {
    Pipeline::registerNode( this, Pipeline::Thread::Game, { QT_STRINGIFY( LibretroVariableModel ) } );
}

void LibretroVariableForwarder::commandIn( Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    switch( command ) {
        case Command::SetLibretroVariable: {
            LibretroVariable variable = data.value<LibretroVariable>();
            QString key = variable.key();
            QStringList values;

            for( QByteArray array : variable.choices() ) {
                values.append( array );
            }

            QString description = variable.description();
            // TODO: Read saved value
            emit insertVariable( key, values, values[ 0 ], description );
            break;
        }

        case Command::Unload: {
            emit clearVariables();
            break;
        }

        default: {
            break;
        }
    }
}

void LibretroVariableForwarder::connectDependencies( QMap<QString, QObject *> objects ) {
    LibretroVariableModel *model = dynamic_cast<LibretroVariableModel *>( objects[ QT_STRINGIFY( LibretroVariableModel ) ] );
    model->setForwarder( this );
    connect( model, &LibretroVariableModel::setVariable, this, &LibretroVariableForwarder::setVariable );
    connect( this, &LibretroVariableForwarder::insertVariable, model, &LibretroVariableModel::insertVariable );
    connect( this, &LibretroVariableForwarder::clearVariables, model, &LibretroVariableModel::clearVariables );
}

void LibretroVariableForwarder::disconnectDependencies( QMap<QString, QObject *> objects ) {
    LibretroVariableModel *model = dynamic_cast<LibretroVariableModel *>( objects[ QT_STRINGIFY( LibretroVariableModel ) ] );
    disconnect( model, &LibretroVariableModel::setVariable, this, &LibretroVariableForwarder::setVariable );
    disconnect( this, &LibretroVariableForwarder::insertVariable, model, &LibretroVariableModel::insertVariable );
    disconnect( this, &LibretroVariableForwarder::clearVariables, model, &LibretroVariableModel::clearVariables );
}

void LibretroVariableForwarder::setVariable( QString key, QString value ) {
    LibretroVariable variable( key.toLatin1() );
    variable.setValue( value.toLatin1() );
    emit commandOut( Command::SetLibretroVariable, QVariant::fromValue<LibretroVariable>( variable ), nodeCurrentTime() );
}
