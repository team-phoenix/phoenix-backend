#include "gamemetadatamodel.h"
#include "gameimporter.h"

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
  connect(&GameImporter::instance(), &GameImporter::updateModel, this,
          &GameMetadataModel::forceUpdate);
  forceUpdate();
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

  if (index.isValid() && index.row() < gameMetadataCache.size()) {
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

// TODO - Update findReleasesBySha1 to use execBatch().
void GameMetadataModel::forceUpdate()
{

  clearCache();

  QList<GameEntry> gameEntries = libraryDb.findAllByGameEntry();

  if (gameEntries.isEmpty()) {
    return;
  }

  beginInsertRows(QModelIndex(), 0, gameEntries.size() - 1);

  gameMetadataCache.resize(gameEntries.size());

  for (int i = 0; i < gameEntries.size(); ++i) {
    const GameEntry &entry = gameEntries.at(i);
    QList<Release> releases = openVgDb.findReleasesBySha1(entry.sha1Checksum);
    Release release;

    if (!releases.isEmpty()) {
      release = releases.first();
    }

    gameMetadataCache[i] = GameMetadata(entry, release);
  }

  endInsertRows();
}

void GameMetadataModel::clearCache()
{
  if (!gameMetadataCache.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, gameMetadataCache.size() - 1);
    endRemoveRows();
    gameMetadataCache.clear();
  }
}
