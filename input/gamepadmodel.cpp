#include "gamepadmodel.h"
#include "gamepad.h"

#include <QQmlEngine>
#include <QDebug>

GamepadModel::GamepadModel(QObject *parent)
    : QAbstractTableModel( parent )
    , m_roleNames {
        { Role::GamepadRole , QByteArrayLiteral( "gamepad" ) },
        { Role::Name , QByteArrayLiteral( "name" ) },
        { Role::Type , QByteArrayLiteral( "type" ) },
        { Role::Mapping , QByteArrayLiteral( "mapping" ) },
     },
      m_keyboard( new Gamepad( -1 ) )
{
    m_gamepadList.append( m_keyboard );
}

GamepadModel::~GamepadModel() {
    delete m_keyboard;
}

QHash<int, QByteArray> GamepadModel::roleNames() const {
    return m_roleNames;
}

int GamepadModel::rowCount(const QModelIndex &) const {
    return m_gamepadList.size();
}

int GamepadModel::columnCount(const QModelIndex &) const {
    return m_roleNames.size();
}

QVariant GamepadModel::data(const QModelIndex &index, int role) const {

    if ( index.isValid() ) {
        switch( role ) {
            case Role::Name:
                return m_gamepadList[ index.row() ]->name();
            case Role::Type:
                return QStringLiteral( "Gamepad" );
            case Role::Mapping:
                return "Unset mapping, needs work";
            case Role::GamepadRole:
                return m_gamepadList[ index.row() ];
            default:
                break;
        }
    }

    return QVariant();
}

QObject *GamepadModel::registerSingletonCallback(QQmlEngine *engine, QJSEngine *) {
    return new GamepadModel( engine );
}

void GamepadModel::addGamepad(const Gamepad *t_gamepad) {
    beginInsertRows( QModelIndex(), m_gamepadList.size(), m_gamepadList.size() );
    m_gamepadList.append( t_gamepad );
    endInsertRows();
}

void GamepadModel::removeGamepad(const Gamepad *t_gamepad) {
    Q_ASSERT( t_gamepad );

    int i=0;
    for ( const auto *_gPad : m_gamepadList ) {
        if ( _gPad == t_gamepad ) {
            m_gamepadList.removeAt( i );
            break;
        }
        ++i;
    }

    beginRemoveRows( QModelIndex(), i, i );
    endRemoveRows();
}
