#include "gamemetadatamodel.h"

#include <QDebug>


GameMetadataModel::GameMetadataModel(QObject* parent)
  : QAbstractTableModel(parent),
    roles(
{
  { Title, "gameTitle"},
  { System, "gameSystem"},
  { Description, "gameDescription"},
  { ImageSource, "gameImageSource"},
})
{
  connect(&gameImporter, &GameImporter::updateModel, this, &GameMetadataModel::forceUpdate);
  forceUpdate();
}

QModelIndex GameMetadataModel::createIndexAt(int row, int column) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex GameMetadataModel::index(int row, int column, const QModelIndex &parent) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column, nullptr) : QModelIndex();
}

int GameMetadataModel::rowCount(const QModelIndex &) const
{
  return gameMetadataCache.size();
}

int GameMetadataModel::columnCount(const QModelIndex &) const
{
  return roles.size();
}

QVariant GameMetadataModel::data(const QModelIndex &index, int role) const
{

  if (index.row() < gameMetadataCache.size()) {
    const GameMetadata &gameMetadata = gameMetadataCache.at(index.row());

    switch (role) {

      case Title:
        return gameMetadata.gameTitle;

      case System:
        return gameMetadata.systemName;

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

QHash<int, QByteArray> GameMetadataModel::roleNames() const
{
  return roles;
}

GameMetadataModel &GameMetadataModel::instance()
{
  static GameMetadataModel metadataModel;
  return metadataModel;
}

void GameMetadataModel::forceUpdate()
{
  clearCache();

  const QList<GameEntry> gameEntries = libraryDb.findAllByGameEntry();

  beginInsertRows(QModelIndex(), 0,
                  gameEntries.size() == 0 ? gameEntries.size() : gameEntries.size() - 1);

  gameMetadataCache.resize(gameEntries.size());

  for (int i = 0; i < gameEntries.size(); ++i) {
    const GameEntry &entry = gameEntries.at(i);
    gameMetadataCache[i] = GameMetadata(entry);
  }

  endInsertRows();
}

void GameMetadataModel::importGames(QList<QUrl> urls)
{
  gameImporter.importGames(urls);
}

void GameMetadataModel::removeGameAt(int index)
{
  beginRemoveRows(QModelIndex(), index, index);

  const GameMetadata &gameMetadata = gameMetadataCache.at(index);
  gameImporter.removeGameBySha1(gameMetadata.gameSha1Checksum);

  endRemoveRows();
}

void GameMetadataModel::clearCache()
{
  if (!gameMetadataCache.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, gameMetadataCache.size() - 1);
    endRemoveRows();
    gameMetadataCache.clear();
  }
}
