#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QModelIndex>
#include <QObject>

#include "remapper.h"

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

class RemapperModel : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY( Remapper *remapper READ getRemapper WRITE setRemapper NOTIFY remapperChanged )

    public:
        explicit RemapperModel( QAbstractListModel *parent = 0 );

        enum RemapperRoles {
            NameRole = Qt::UserRole + 1,
        };

        // FIXME: Move to cpp before committing
        QHash<int, QByteArray> roleNames() {
            QHash<int, QByteArray> roles;
            roles[ NameRole ] = "name";
            return roles;
        }

        // FIXME: Move to cpp before committing
        // One row per GUID
        int rowCount( const QModelIndex &/*parent = QModelIndex()*/ ) const override {
            return 0;
        }

        // FIXME: Move to cpp before committing
        QVariant data( const QModelIndex &/*index*/, int /*role = Qt::DisplayRole*/ ) const override {
            return QVariant();
        }

    signals:
        void remapperChanged();

    public slots:
        // A new controller GUID was seen, add to the model's list
        void controllerAdded( QString /*GUID*/ ){}

        // The last remaining controller with this GUID was removed, do not accept remap requests for this one
        void controllerRemoved( QString /*GUID*/ ){}

        // Only fired while not in remap mode, true if any button on any controller with this GUID has been pressed
        void buttonUpdate( QString /*GUID*/, bool /*pressed*/ ){}

        // Remap mode completed, this GUID now gets this button assigned to it
        void remapModeEnd( QString /*GUID*/, QString /*originalButton*/, QString /*remappedButton*/ ){}

    private:
        // FIXME: Move to cpp before committing
        Remapper *getRemapper() { return nullptr; }
        void setRemapper( Remapper */*remapper*/ ) {}

        // Copy of the remapping data
        // GUID : (button : remapped button)
        QMap<QString, QMap<QString, QString>> remapData;
};
