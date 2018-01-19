#pragma once

#include "database.h"
#include "gameentry.h"
#include "systemcoremap.h"

#include <QMutex>

class LibraryDb : public Database
{
public:
  LibraryDb();
  ~LibraryDb() = default;

  QList<QPair<GameEntry, SystemCoreMap>> findAllByGameEntry();
  QList<QPair<GameEntry, SystemCoreMap>> findAllByGameEntryFilterBySystem(QString systemFullName);
  QStringList getTableNames() const;

  void insert(GameEntry entry);
  void insert(SystemCoreMap systemCoreMap);

  void removeBySha1(QString sha1);
  void removeAllGameEntries();
  void dropAllTables();

private:
  bool createSchema(QSqlDatabase &db);

  QStringList tableNames;
};
