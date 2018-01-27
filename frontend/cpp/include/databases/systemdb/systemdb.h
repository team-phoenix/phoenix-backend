#pragma once

#include "database.h"
#include "coresystemmap.h"
#include "systementity.h"

#include <QSqlDatabase>

class SystemDb : public Database
{
public:
  SystemDb();

//  QList<CoreSystemMap> findAllByCoreSystemMap()
//  {
//    return findAllBy<CoreSystemMap>( "coreSystemMap");
//  }

  QString findCoreWhereSystemFullNameIs(QString systemFullName);

  QList<SystemEntity> findAllBySystem();

  bool createSchema(QSqlDatabase &db);
};
