#pragma once

#include "database.h"
#include "gameentry.h"

#include <QMutex>

class LibraryDb : public Database
{
public:
  LibraryDb();
  ~LibraryDb() = default;

  QList<GameEntry> findAllByGameEntry();
  QStringList getTableNames() const;

  void insert(GameEntry entry);
  void removeBySha1(QString sha1);
  void removeAllGameEntries();
  void dropAllTables();

private:
  bool createSchema(QSqlDatabase &db);

  QStringList tableNames;
};
