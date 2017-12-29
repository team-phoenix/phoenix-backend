#pragma once

#include "gameentry.h"

#include <QAbstractTableModel>
#include <QHash>

class GameEntryDbModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit GameEntryDbModel(QObject* parent = nullptr);

  enum Roles {
    Title,
    Description,
    ImageSource,
  };

  QModelIndex index(int row, int column, const QModelIndex &parent) const override;

  int rowCount(const QModelIndex &) const override;

  int columnCount(const QModelIndex &) const override;

  QVariant data(const QModelIndex &index, int role) const override;

  QHash<int, QByteArray> roleNames() const override;

private:
  QHash<int, QByteArray> roles;
  QVector<GameEntry> gameMetadataCache;
};
