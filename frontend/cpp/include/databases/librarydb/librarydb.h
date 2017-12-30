#pragma once

#include "database.h"
#include "gameentry.h"

class LibraryDb : public Database
{
public:
  LibraryDb();
  ~LibraryDb() = default;

  QList<GameEntry> findAllByGameEntry();

  void insert(GameEntry entry);
  void removeAllGameEntries();

private:
  bool createSchema(QSqlDatabase &db);
};
