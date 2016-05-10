#include "libretrovariablemodel.h"
#include "libretrovariableforwarder.h"
#include "node.h";

#include <QDebug>

LibretroVariableModel::LibretroVariableModel(QObject *parent)
    : QAbstractTableModel( parent ),
      m_roleNames {
          { Role::Key, QByteArrayLiteral( "key" ) },
          { Role::Choices, QByteArrayLiteral( "choices" ) },
          { Role::Description, QByteArrayLiteral( "description" ) },
      }
{

}

QHash<int, QByteArray> LibretroVariableModel::roleNames() const {
    return m_roleNames;
}

int LibretroVariableModel::rowCount( const QModelIndex & ) const {
    return m_varList.size();
}

int LibretroVariableModel::columnCount( const QModelIndex & ) const {
    return m_roleNames.size();
}

QVariant LibretroVariableModel::data(const QModelIndex &index, int role) const
{
    if ( index.isValid() ) {

        switch( role ) {
            case Role::Key:
                return QString( m_varList[ index.row() ].key() );
            case Role::Choices: {
                QStringList _result;
                for ( const QByteArray &bytes : m_varList[ index.row() ].choices() ) {
                    _result.append( bytes );
                }
                return _result;
            }
            case Role::Description:
                return QString( m_varList[ index.row() ].description() );
            default:
                break;
        }

    }

    return QVariant();
}

void LibretroVariableModel::setForwarder( LibretroVariableForwarder *t_forwarder ) {
    m_forwarder = t_forwarder;
    connect( m_forwarder, &LibretroVariableForwarder::variableFound, this, &LibretroVariableModel::appendVariable );
}

void LibretroVariableModel::appendVariable( LibretroVariable t_var ) {
    beginInsertRows( QModelIndex(), m_varList.size(), m_varList.size() );
    m_varList.append( t_var );
    endInsertRows();
}

void LibretroVariableModel::removeVariable( LibretroVariable &t_var) {
    for ( int i=0; i < m_varList.size(); ++i ) {
        if ( t_var == m_varList[ i ] ) {
            beginRemoveRows( QModelIndex(), i, i );
            endRemoveRows();
            break;
        }
    }
}

void LibretroVariableModel::updateVariable( QString t_key, QString t_value ) {
    const QByteArray _keyBytes = t_key.toUtf8();
    for ( int i=0; i < m_varList.size(); ++i ) {
        LibretroVariable variable = m_varList[ i ];
        if ( _keyBytes == variable.key() ) {
            variable.setValue( t_value.toUtf8() );
            QVariant var;
            var.setValue( variable );
            m_forwarder->commandIn( Node::Command::SetLibretroVariable, var, -1 );

            break;
        }
    }
}
