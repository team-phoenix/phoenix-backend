#pragma once

#include <QSqlDatabase>

class GameMetadataDB
{
public:
  QSqlDatabase getDatabase() { return QSqlDatabase(); }
};
