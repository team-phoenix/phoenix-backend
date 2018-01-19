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
  { AbsoluteGamePath, "gameAbsoluteFilePath"},
  { AbsoluteCorePath, "coreAbsoluteFilePath"},
})
{
  connect(&gameImporter, &GameImporter::updateModel, this, &GameMetadataModel::forceUpdate);
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

      case AbsoluteGamePath:
        return gameMetadata.gameFilePath;

      case AbsoluteCorePath:
        return gameMetadata.userSetCore.isEmpty() ? gameMetadata.defaultCore : gameMetadata.userSetCore;

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
  const QList<QPair<GameEntry, SystemCoreMap>> gameEntriesSystemPair = libraryDb.findAllByGameEntry();
  fillCache(gameEntriesSystemPair);
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

void GameMetadataModel::filterBySystem(QString fullSystemName)
{
  clearCache();

  QList<QPair<GameEntry, SystemCoreMap>> gameEntriesSystemPair;

  if (fullSystemName.toLower() == "all") {
    gameEntriesSystemPair = libraryDb.findAllByGameEntry();
  } else {
    gameEntriesSystemPair = libraryDb.findAllByGameEntryFilterBySystem(fullSystemName);
  }

  fillCache(gameEntriesSystemPair);
}

void GameMetadataModel::clearCache()
{
  if (!gameMetadataCache.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, gameMetadataCache.size() - 1);
    endRemoveRows();
    gameMetadataCache.clear();
  }
}

void GameMetadataModel::fillCache(const QList<QPair<GameEntry, SystemCoreMap>> &gameEntries)
{
  if (gameEntries.isEmpty()) {
    return;
  }

  beginInsertRows(QModelIndex(), 0, gameEntries.size() - 1);

  gameMetadataCache.resize(gameEntries.size());

  for (int i = 0; i < gameEntries.size(); ++i) {
    const QPair<GameEntry, SystemCoreMap> &pair = gameEntries.at(i);
    const GameEntry entry = pair.first;
    const SystemCoreMap systemCoreMap = pair.second;

    gameMetadataCache[i] = GameMetadata(entry, systemCoreMap);
  }

  endInsertRows();
}
