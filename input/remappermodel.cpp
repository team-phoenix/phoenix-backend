#include "remappermodel.h"
#include "remapper.h"


RemapperModel::RemapperModel( QAbstractListModel *parent ) : QAbstractListModel( parent ) {
}

// Public

QHash<int, QByteArray> RemapperModel::roleNames() const {
    return {
        { GUIDRole, QByteArrayLiteral( "GUID" ) },
        { RemapDataRole, QByteArrayLiteral( "remapData" ) },
        { AvailableRole, QByteArrayLiteral( "available" ) },
        { PressedRole, QByteArrayLiteral( "pressed" ) },
        { FriendlyNameRole, QByteArrayLiteral( "friendlyName" ) },
    };
}

QVariant RemapperModel::data( const QModelIndex &index, int role ) const {
    // Sanity checks
    {
        if( remapData.isEmpty() ) {
            return QVariant();
        }

        if( index.row() < 0 || index.row() >= remapData.size() ) {
            return QVariant();
        }
    }

    switch( role ) {
        case GUIDRole: {
            QString GUID = rowToGUID( index.row() );
            return GUID;
        }

        case RemapDataRole: {
            // Get the GUID
            QString GUID = rowToGUID( index.row() );

            // Get the remap data
            QVariantMap remapData;

            for( QString key : this->remapData[ GUID ].keys() ) {
                remapData[ key ] = this->remapData[ GUID ][ key ];
            }

            return remapData;
        }

        case AvailableRole: {
            return available[ rowToGUID( index.row() ) ];
        }

        case PressedRole: {
            return pressed[ rowToGUID( index.row() ) ];
        }

        case FriendlyNameRole: {
            return friendlyNames[ rowToGUID( index.row() ) ];
        }

        default:
            return QVariant();
    }
}

int RemapperModel::rowCount( const QModelIndex & ) const {
    return remapData.size();
}

// Public slots

void RemapperModel::controllerAdded( QString GUID, QString friendlyName ) {
    insertRowIfNotPresent( GUID );
    available[ GUID ] = true;
    friendlyNames[ GUID ] = friendlyName;
    int row = GUIDToRow( GUID );
    emit dataChanged( createIndex( row, 0 ), createIndex( row, 0 ), { AvailableRole, FriendlyNameRole } );
}

void RemapperModel::controllerRemoved( QString GUID ) {
    available[ GUID ] = false;
    int row = GUIDToRow( GUID );
    emit dataChanged( createIndex( row, 0 ), createIndex( row, 0 ), { AvailableRole } );
}

void RemapperModel::buttonUpdate( QString GUID, bool pressed ) {
    this->pressed[ GUID ] = pressed;
    int row = GUIDToRow( GUID );
    emit dataChanged( createIndex( row, 0 ), createIndex( row, 0 ), { PressedRole } );
}

void RemapperModel::remappingEnded() {
    remapMode = false;
    emit remapModeChanged();
}

void RemapperModel::setMapping( QString GUID, QString virtualButton, QString physicalButton ) {
    // Update the model if this GUID is not in the list yet
    insertRowIfNotPresent( GUID );

    // Add to/update the map
    // FIXME: This is backwards compared to how it's stored in Remapper
    remapData[ GUID ][ virtualButton ] = physicalButton;
    int row = GUIDToRow( GUID );
    emit dataChanged( createIndex( row, 0 ), createIndex( row, 0 ), { RemapDataRole } );
}

void RemapperModel::setRemapper( Remapper *t_remapper ) {
    connect( t_remapper, &Remapper::controllerAdded, this, &RemapperModel::controllerAdded );
    connect( t_remapper, &Remapper::controllerRemoved, this, &RemapperModel::controllerRemoved );
    connect( t_remapper, &Remapper::buttonUpdate, this, &RemapperModel::buttonUpdate );
    connect( t_remapper, &Remapper::remappingEnded, this, &RemapperModel::remappingEnded );
    connect( t_remapper, &Remapper::setMapping, this, &RemapperModel::setMapping );
    connect( this, &RemapperModel::beginRemapping, t_remapper, &Remapper::beginRemapping );
}

void RemapperModel::beginRemapping( QString GUID, QString button ) {
    if( !remapMode ) {
        remapMode = true;
        emit remapModeChanged();
        emit beginRemapping( GUID, button );
    } else {
        qCWarning( phxInput ) << "Remap mode already active!";
    }
}

// Helpers

bool RemapperModel::insertRowIfNotPresent( QString GUID ) {
    if( !remapData.contains( GUID ) ) {
        // Find this GUID's position in the sorted list
        int row = GUIDToRow( GUID );

        // Place into list
        beginInsertRows( QModelIndex(), row, row );
        remapData.insert( GUID, QStringMap() );
        endInsertRows();
    }

    return false;
}

int RemapperModel::GUIDToRow( QString GUID ) const {
    return std::distance( remapData.begin(), remapData.lowerBound( GUID ) );
}

QString RemapperModel::rowToGUID( int row ) const {
    return remapData.keys()[ row ];
}
