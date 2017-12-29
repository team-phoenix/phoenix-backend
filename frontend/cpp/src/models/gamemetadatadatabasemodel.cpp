#include "gamemetadatadatabasemodel.h"

#include <QDebug>

GameMetadataDatabaseModel::GameMetadataDatabaseModel(QObject* parent)
  : QAbstractTableModel(parent),
    roles(
{
  { Title, "gameTitle"},
  { Description, "gameDescription"},
  { ImageSource, "gameImageSource"},
})
{

  const int itemsAdded = 3;
  beginInsertRows(QModelIndex(), gameMetadataCache.size(), gameMetadataCache.size() + itemsAdded - 1);

  gameMetadataCache.append(GameMetadata{ "Supra Title", "Blah blah", "/path/to/icon"});
  gameMetadataCache.append(GameMetadata{ "Supra MMEME", "Blah blahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblah", "/path/to/icon"});
  gameMetadataCache.append(GameMetadata{ "Supra Tfe", "Blah blah", "/path/to/icon"});

  endInsertRows();
}

QModelIndex GameMetadataDatabaseModel::index(int row, int column, const QModelIndex &parent) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column, nullptr) : QModelIndex();
}

int GameMetadataDatabaseModel::rowCount(const QModelIndex &) const
{
  return gameMetadataCache.size();
}

int GameMetadataDatabaseModel::columnCount(const QModelIndex &) const
{
  return roles.size();
}

QVariant GameMetadataDatabaseModel::data(const QModelIndex &index, int role) const
{

  if (index.isValid() && index.row() < gameMetadataCache.size()) {
    const GameMetadata &gameMetadata = gameMetadataCache.at(index.row());

    switch (role) {

      case Title:
        return gameMetadata.gameTitle;

      case Description:
        return gameMetadata.gameDescription;

      case ImageSource:
        return gameMetadata.gameImageSource;

      default:
        break;
    }
  }

  return QVariant();
}

QHash<int, QByteArray> GameMetadataDatabaseModel::roleNames() const
{
  return roles;
}
