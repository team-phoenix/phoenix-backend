#pragma once

#include "gamemetadata.h"
#include "librarydb.h"
#include "openvgdb.h"

#include <QAbstractTableModel>
#include <QHash>

class GameMetadataModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit GameMetadataModel(QObject* parent = nullptr);

  enum Roles {
    Title,
    System,
    Description,
    ImageSource,
  };

  QModelIndex index(int row, int column, const QModelIndex &parent) const override;

  int rowCount(const QModelIndex &) const override;

  int columnCount(const QModelIndex &) const override;

  QVariant data(const QModelIndex &index, int role) const override;

  QHash<int, QByteArray> roleNames() const override;

  void forceUpdate();

private:
  QHash<int, QByteArray> roles;
  QVector<GameMetadata> gameMetadataCache;
  LibraryDb libraryDb;
  OpenVgDb openVgDb;
};
