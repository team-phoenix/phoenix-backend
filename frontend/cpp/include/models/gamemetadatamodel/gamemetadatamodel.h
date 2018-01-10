#pragma once

#include "gamemetadata.h"
#include "librarydb.h"
#include "openvgdb.h"
#include "gameimporter.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QDebug>

class GameMetadataModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  ~GameMetadataModel() = default;

  enum Roles {
    Title = Qt::UserRole + 1,
    System,
    Description,
    ImageSource,
  };

  QModelIndex createIndexAt(int row, int column) const;

  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  virtual QVariant data(const QModelIndex &index, int role) const override;

  virtual QHash<int, QByteArray> roleNames() const override;

  static GameMetadataModel &instance();

public slots:
  virtual void forceUpdate();
  void importGames(QList<QUrl> urls);
  void removeGameAt(int index);

private:
  void clearCache();

private:
  explicit GameMetadataModel(QObject* parent = nullptr);

  QHash<int, QByteArray> roles;
  QVector<GameMetadata> gameMetadataCache;

  LibraryDb libraryDb;
  OpenVgDb openVgDb;
  GameImporter gameImporter;
};
