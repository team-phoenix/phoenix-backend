#pragma once

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

#include <iterator>

#include "pipelinecommon.h"

/*
 * RemapperModel is a QML model whose job is to relay information between the QML world and Remapper, which is part
 * of the global pipeline. It provides a list of all controllers currently connected (indexed internally by joystickID)
 * with the following roles:
 * name (QString): Name of the controller. By default it will use the SDL-provided name.
 * GUID (QString): Controller GUID
 * pressed (bool): True if any button on this controller is currently being pressed. Used so the user can tell controllers
 *                 of the same model apart.
 * remapData (map): Map of string:string, key is the button name and the value is the remapped name
 */

class Remapper;

class RemapperModel : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY( bool remapMode MEMBER remapMode NOTIFY remapModeChanged )

    public:
        explicit RemapperModel( QAbstractListModel *parent = nullptr );

        enum RemapperRoles {
            // GUID: String
            // GUID of the controller
            GUIDRole = Qt::UserRole + 1,

            // remapData: Object( String:String )
            // Remap data for this GUID ( button:button )
            RemapDataRole,

            // available: Boolean
            // True iff a controller is plugged in for this particular GUID
            AvailableRole,

            // pressed: Boolean
            // True iff any controller with this GUID has any button pressed
            PressedRole,

            // friendlyName: String
            // Friendly name of the controller as provided by SDL
            FriendlyNameRole,
        };

        QHash<int, QByteArray> roleNames() const override;

        QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

        // One row per GUID
        // Ignore parent, this isn't a table
        int rowCount( const QModelIndex &/*parent = QModelIndex()*/ ) const override;

    signals:
        void remapModeChanged();
        void remapModeBegin( QString GUID, QString button );

    public slots:
        // A new controller GUID was seen, add to the model's list
        void controllerAdded( QString GUID , QString friendlyName );

        // The last remaining controller with this GUID was removed, do not accept remap requests for this one
        void controllerRemoved( QString GUID );

        // Only fired while not in remap mode, true if any button on any controller with this GUID has been pressed
        void buttonUpdate( QString GUID, bool pressed );

        // Remap mode completed, update UI
        void remapModeEnd();

        // Update remapData, reset model
        void remapUpdate( QString GUID, QString originalButton, QString remappedButton );

        // Connect to this Remapper
        void setRemapper( Remapper *t_remapper );

        // Call from QML
        void beginRemap( QString GUID, QString button );

    private:
        // Are we currently remapping?
        bool remapMode{ false };

        // Copy of the remapping data
        // GUID : (button : remapped button)
        QMap<QString, QStringMap> remapData;

        // True if any controller with a particular GUID has any button pressed
        // GUID : button pressed
        QMap<QString, bool> pressed;

        // True if any controller is plugged in with this GUID
        QMap<QString, bool> available;

        // Friendly names for controllers as provided by SDL
        QStringMap friendlyNames;

        // Helpers

        bool insertRowIfNotPresent( QString GUID );

        // Returns the next highest GUID if the given one does not exist
        int GUIDToRow( QString GUID ) const;

        QString rowToGUID( int row ) const;
};
