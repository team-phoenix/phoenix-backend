#include "libretrovariablemodel.h"

LibretroVariableModel::LibretroVariableModel(QObject *parent)
    : QAbstractTableModel( parent ) {

}

QHash<int, QByteArray> LibretroVariableModel::roleNames() const {
    return {
        { Role::Key, QByteArrayLiteral( "key" ) },
        { Role::Value, QByteArrayLiteral( "value" ) },
    };
}

int LibretroVariableModel::rowCount( const QModelIndex & ) const {
    return m_varList.size();
}

int LibretroVariableModel::columnCount( const QModelIndex & ) const {
    return 2;
}

QVariant LibretroVariableModel::data(const QModelIndex &index, int role) const
{
    if ( index.isValid() ) {

        switch( role ) {
        case Role::Key:
            return QString::fromStdString( m_varList[ index.row() ].key() );
        case Role::Value:
            return QString::fromStdString( m_varList[ index.row() ].value() );
        default:
            break;
        }

    }

    return QVariant();
}

void LibretroVariableModel::appendVariable(const LibretroVariable &t_var) {
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
