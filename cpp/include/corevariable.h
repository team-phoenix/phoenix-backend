#pragma once

#include <QByteArray>
#include <QVector>
#include <QJsonObject>

#include "macros.h"

#include <QHash>

class QDebug;

class CoreVariable {
public:

    CoreVariable() = default;

    explicit CoreVariable( QByteArray &&t_key, QByteArray &&t_value );
    explicit CoreVariable( const QByteArray &t_key, const QByteArray &t_value );


    GET_SET( CoreVariable, QByteArray, key )
    GET_SET( CoreVariable, QByteArray, description )
    GET_SET( CoreVariable, QVector<QByteArray>, values )
    GET_SET( CoreVariable, QByteArray, currentValue )

    CoreVariable& values( const QByteArray &t_val );

    operator QJsonObject() const;
};

class VariableModel {

public:
    VariableModel()
    {
    }

    bool contains( const char *t_key ) {
        return m_variables.contains( t_key );
    }

    const QByteArray &currentValue( const char *t_key ) {
        return m_variables[ t_key ].currentValue();
    }

    void insert( const char *t_key, CoreVariable &&t_var );

    void insert( const QByteArray &t_key, const QByteArray &t_value );


public:
    QHash<QByteArray, CoreVariable>::iterator begin() {
        return m_variables.begin();
    }

    QHash<QByteArray, CoreVariable>::const_iterator cbegin() {
        return m_variables.cbegin();
    }

    QHash<QByteArray, CoreVariable>::iterator end() {
        return m_variables.end();
    }

    QHash<QByteArray, CoreVariable>::const_iterator cend() {
        return m_variables.cend();
    }

private:
    QHash<QByteArray, CoreVariable> m_variables;
};


QDebug &operator<< ( QDebug &debug, const CoreVariable &t_var );
QDebug &operator<< ( QDebug &debug, CoreVariable &&t_var );
