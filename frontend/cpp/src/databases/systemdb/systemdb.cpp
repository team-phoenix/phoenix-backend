#include "systemdb.h"

#include <QCoreApplication>

SystemDb::SystemDb()
  : Database(QCoreApplication::applicationDirPath()  + "/databases/systemdb.sqlite")
{

  QSqlDatabase db = databaseConnection();
  createSchema(db);
}

QString SystemDb::findCoreWhereSystemFullNameIs(QString systemFullName)
{
  QSqlDatabase db = databaseConnection();
  QSqlQuery query(db);

  query.prepare("SELECT cores.coreName FROM cores "
                "INNER JOIN coreSystemMapping "
                "ON cores.coreID = coreSystemMapping.coreID "
                "INNER JOIN systems "
                "ON systems.systemID = coreSystemMapping.systemID "
                "WHERE systems.systemFullName = :systemName");

  query.bindValue(":systemName", systemFullName);

  if (query.exec()) {
    while (query.next()) {
      return query.value(0).toString();
    }
  }

}

QList<SystemEntity> SystemDb::findAllBySystem()
{
  return findAllBy<SystemEntity>("systems", "*");
}

bool SystemDb::createSchema(QSqlDatabase &db)
{
  QSqlQuery query(db);

  struct SystemCoreEntity {
    QString systemFullName;
    QString systemShortName;
    QString coreName;
  };

  const QList<SystemCoreEntity> systemPairs{
    SystemCoreEntity{"Nintendo Entertainment System", "NES", "nestopia_libretro"},
    SystemCoreEntity{"Nintendo Super Nintendo Entertainment System", "SNES", "snes9x_libretro"},
    SystemCoreEntity{"Sony PlayStation", "PSX", "mednafen_psx_libretro"},
    SystemCoreEntity{"Sega Genesis/Mega Drive", "Genesis", "yabause_libretro"},
  };

  if (query.exec("CREATE TABLE systems (\n "
                 "systemID INTEGER PRIMARY KEY, "
                 "systemFullName TEXT, "
                 "systemShortName TEXT "
                 ");")) {

    int i = 0;

    for (const SystemCoreEntity &entity : systemPairs) {
      query.prepare("INSERT INTO systems (systemID, systemFullName, systemShortName) VALUES(:id, :fullName, :shortName)");

      query.bindValue(":id", i);
      query.bindValue(":fullName", entity.systemFullName);
      query.bindValue(":shortName", entity.systemShortName);
      query.exec();
      ++i;
    }
  }

  if (query.exec("CREATE TABLE cores (\n "
                 "coreID INTEGER PRIMARY KEY, "
                 "coreName TEXT "
                 ");")) {



    int i = 0;

    for (const SystemCoreEntity &entity : systemPairs) {
      query.prepare("INSERT INTO cores (coreID, coreName) VALUES(:id, :core)");
      query.bindValue(":id", i);
      query.bindValue(":core", entity.coreName);

      query.exec();
      ++i;
    }
  }

  if (query.exec("CREATE TABLE coreSystemMapping (\n "
                 "coreID INTEGER NOT NULL, "
                 "systemID INTEGER NOT NULL, "
                 "FOREIGN KEY (coreID) REFERENCES cores ON DELETE CASCADE, \n"
                 "FOREIGN KEY (systemID) REFERENCES systems ON DELETE CASCADE"
                 ");")) {


    for (int i = 0; i < systemPairs.size(); ++i) {
      query.prepare("INSERT INTO coreSystemMapping (coreID, systemID) VALUES(:coreID, :systemID)");
      query.bindValue(":coreID", i);
      query.bindValue(":systemID", i);

      query.exec();
    }

  }

  return db.commit();
}
