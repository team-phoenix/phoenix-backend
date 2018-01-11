#pragma once

#include "librarydb.h"

#include <QFile>
#include <QDebug>

class TempDbSession
{
public:
  TempDbSession(LibraryDb* db)
  {
    database = db;
  }
  ~TempDbSession()
  {
    database->dropAllTables();
  }

private:
  LibraryDb* database;
};
