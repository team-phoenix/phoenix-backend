#include "remapmodel.h"
#include "gamepad.h"

#include <QQmlEngine>
#include <QDebug>

RemapModel::RemapModel(QObject *parent)
    : QAbstractListModel( parent )
{
    m_roleNames[ Roles::ButtonKey ] = "buttonKey";
    m_roleNames[ Roles::ButtonValue ] = "buttonValue";

//    for ( int i=0; i < static_cast<int>( Gamepad::Button::Max ); ++i ) {
//        m_roleNames[ i + ( Qt::UserRole + 1 ) ] = toString( static_cast<Gamepad::Button>( i ) ).toLocal8Bit();
//    }
    m_gamepadList.append( new Gamepad( -1 ) );

}

QHash<int, QByteArray> RemapModel::roleNames() const {
    return m_roleNames;
}

int RemapModel::rowCount(const QModelIndex &) const {
    return m_gamepadKeys.size() ;
}

QVariant RemapModel::data(const QModelIndex &index, int role) const {

    if ( index.isValid() ) {
        switch (role) {
            case Roles::ButtonKey:
                return m_gamepadKeys.isEmpty() ? QVariant() : m_gamepadKeys[ index.row() ];
            case Roles::ButtonValue:
                return m_gamepadValues.isEmpty() ? QVariant() : m_gamepadValues[ index.row() ];
            default:
                break;
        }
    }

    return QVariant();
}

QObject *RemapModel::registerSingletonCallback(QQmlEngine *engine, QJSEngine *) {
    return new RemapModel( engine );
}

void RemapModel::addGamepad(const Gamepad *t_gamepad) {
    m_gamepadList.append( t_gamepad );
    //currentIndex( 0 );
}

void RemapModel::removeGamepad(const Gamepad *t_gamepad) {
    Q_ASSERT( t_gamepad );
    beginRemoveRows( QModelIndex(), m_gamepadKeys.size(), m_gamepadKeys.size() );
    //m_gamepadList
    endRemoveRows();
}

void RemapModel::currentIndex(int _index) {

    if ( m_gamepadList.isEmpty() ) {
        return;
    }

    Q_ASSERT( m_gamepadList[ _index ] );
    auto mapping = m_gamepadList[ _index ]->mapping();
    beginInsertRows( QModelIndex(), 0, mapping.size() );
    for ( const auto &key : mapping ) {
        m_gamepadKeys.append( key );
        m_gamepadValues.append( mapping[ key ] );
    }

    endInsertColumns();
}
