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

  void insert(GameEntry entry);
  void removeBySha1(QString sha1);
  void removeAllGameEntries();

private:
  bool createSchema(QSqlDatabase &db);
};
