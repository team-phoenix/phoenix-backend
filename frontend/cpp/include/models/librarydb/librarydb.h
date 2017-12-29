#pragma once

#include "database.h"
#include "gameentry.h"
#include <QDebug>

class LibraryDb : public Database
{
public:
  LibraryDb();

  QList<GameEntry> findAllByGameEntry();

  void insert(GameEntry entry);

private:
  bool createSchema(QSqlDatabase &db);
};
