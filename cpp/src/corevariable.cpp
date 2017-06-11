#include "corevariable.h"

#include <QJsonArray>

#include <QDebug>
#include <QDebugStateSaver>

CoreVariable::CoreVariable(QByteArray &&t_key, QByteArray &&t_value)
    : CoreVariable( t_key, t_value )
{

}

CoreVariable::CoreVariable(const QByteArray &t_key, const QByteArray &t_value)
{

    if ( t_value.contains( ';' ) ) {

        QList<QByteArray> splitValue = t_value.split( ';' );
        Q_ASSERT( splitValue.size() == 2 );

        key( t_key.simplified() );
        description( splitValue[ 0 ].simplified() );

        for ( const QByteArray &vals : splitValue[ 1 ].split( '|' ) ) {
            values( vals.simplified() );
        }

    }

}

CoreVariable &CoreVariable::values(const QByteArray &t_val) {
    m_values.append( t_val );
    return *this;
}

CoreVariable::operator QJsonObject() const {

    QJsonObject root {
        { "response", "setVariable" },
    };


    QJsonArray variables;

    for ( const QByteArray &var : m_values ) {
        variables.append( QString( var ) );
    }

    root[ "key" ] = QString( key() );
    root[ "variables" ] = variables;

    return root;

}


QDebug &operator<<(QDebug &debug, const CoreVariable &t_var) {
    QDebugStateSaver saver( debug );
    Q_UNUSED( saver );

    debug.nospace() << "CoreVariable{ \n\t"
                    << t_var.key() << ",\n\t"
                    << t_var.description() << ",\n\t"
                    << t_var.values() << " }";

    return debug;
}

QDebug &operator<<(QDebug &debug, CoreVariable &&t_var) {
    return operator <<( debug, t_var );
}

void VariableModel::insert(const char *t_key, CoreVariable &&t_var) {

    m_variables.insert( t_key, t_var );
}

void VariableModel::insert(const QByteArray &t_key, const QByteArray &t_value) {

    if ( m_variables.contains( t_key ) ) {

        m_variables[ t_key ].currentValue( t_value );

    }

}
