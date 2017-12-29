#include "gameentrydbmodel.h"

#include <QDebug>

GameEntryDbModel::GameEntryDbModel(QObject* parent)
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

//  gameMetadataCache.append(GameEntry{ "Supra Title", "Blah blah", "/path/to/icon"});
//  gameMetadataCache.append(GameEntry{ "Supra MMEME", "Blah blahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblahblah", "/path/to/icon"});
//  gameMetadataCache.append(GameEntry{ "Supra Tfe", "Blah blah", "/path/to/icon"});

  endInsertRows();
}

QModelIndex GameEntryDbModel::index(int row, int column, const QModelIndex &parent) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column, nullptr) : QModelIndex();
}

int GameEntryDbModel::rowCount(const QModelIndex &) const
{
  return gameMetadataCache.size();
}

int GameEntryDbModel::columnCount(const QModelIndex &) const
{
  return roles.size();
}

QVariant GameEntryDbModel::data(const QModelIndex &index, int role) const
{

//  if (index.isValid() && index.row() < gameMetadataCache.size()) {
//    const GameEntry &gameMetadata = gameMetadataCache.at(index.row());

//    switch (role) {

//      case Title:
//        return gameMetadata.gameTitle;

//      case Description:
//        return gameMetadata.gameDescription;

//      case ImageSource:
//        return gameMetadata.gameImageSource;

//      default:
//        break;
//    }
//  }

  return QVariant();
}

QHash<int, QByteArray> GameEntryDbModel::roleNames() const
{
  return roles;
}
