
#include "librarydb.h"
#include "logger.h"

#include <QCoreApplication>
#include <QStringBuilder>
#include <QSqlQuery>
#include <QDebug>

#include <QMutex>
#include <QMutexLocker>

static QMutex* dbMutex = new QMutex();

LibraryDb::LibraryDb()
  : Database(QCoreApplication::applicationDirPath()  + "/databases/librarydb.sqlite")
{
  QSqlDatabase db = databaseConnection();
  tableNames = QStringList({ "games", "collections", "collectionMappings", "system_to_core_mapping"});
  createSchema(db);
}

QList<QPair<GameEntry, SystemCoreMap>> LibraryDb::findAllByGameEntry()
{
  QMutexLocker locker(dbMutex);

  QSqlDatabase db = databaseConnection();
  QSqlQuery query = QSqlQuery(db);

  QList<QPair<GameEntry, SystemCoreMap>> result;

  if (query.exec(QString("SELECT DISTINCT "
                         "games.timePlayed, "
                         "games.gameTitle, "
                         "games.gameImageSource, "
                         "games.absoluteFilePath, "
                         "games.gameDescription, "
                         "games.systemFullName, "
                         "games.userSetCore, "
                         "games.sha1Checksum, "
                         "system_to_core_mapping.systemFullName, "
                         "system_to_core_mapping.coreName, "
                         "system_to_core_mapping.isDefault "
                         "FROM games "
                         "INNER JOIN system_to_core_mapping "
                         "ON system_to_core_mapping.systemFullName = games.systemFullName "
                         "WHERE system_to_core_mapping.isDefault = 1"))) {
    while (query.next()) {

      GameEntry gameEntry;
      gameEntry.timePlayed = query.value(0).toDateTime();
      gameEntry.gameTitle = query.value(1).toString();
      gameEntry.gameImageSource = query.value(2).toString();
      gameEntry.absoluteFilePath = query.value(3).toString();
      gameEntry.gameDescription = query.value(4).toString();
      gameEntry.systemFullName = query.value(5).toString();
      gameEntry.userSetCore = query.value(6).toString();
      gameEntry.sha1Checksum = query.value(7).toString();

      SystemCoreMap systemCoreMap;
      systemCoreMap.systemFullName = query.value(8).toString();
      systemCoreMap.coreName = query.value(9).toString();
      systemCoreMap.isDefault = query.value(10).toInt() == 0 ? false : true;

      result.append(QPair<GameEntry, SystemCoreMap>(gameEntry, systemCoreMap));
    }
  } else {
    qDebug() << query.lastError().text();
  }

  return result;

}

QList<QPair<GameEntry, SystemCoreMap>> LibraryDb::findAllByGameEntryFilterBySystem(
                                      QString systemFullName)
{
  QMutexLocker locker(dbMutex);
  QSqlDatabase db = databaseConnection();
  QSqlQuery query = QSqlQuery(db);

  QList<QPair<GameEntry, SystemCoreMap>> result;

  query.prepare("SELECT DISTINCT "
                "games.timePlayed, "
                "games.gameTitle, "
                "games.gameImageSource, "
                "games.absoluteFilePath, "
                "games.gameDescription, "
                "games.systemFullName, "
                "games.userSetCore, "
                "games.sha1Checksum, "
                "system_to_core_mapping.systemFullName, "
                "system_to_core_mapping.coreName, "
                "system_to_core_mapping.isDefault "
                "FROM games "
                "INNER JOIN system_to_core_mapping "
                "ON system_to_core_mapping.systemFullName = games.systemFullName "
                "WHERE games.systemFullName = :system "
                "AND system_to_core_mapping.isDefault = 1");

  query.bindValue(":system", systemFullName);

  if (query.exec()) {
    while (query.next()) {

      GameEntry gameEntry;
      gameEntry.timePlayed = query.value(0).toDateTime();
      gameEntry.gameTitle = query.value(1).toString();
      gameEntry.gameImageSource = query.value(2).toString();
      gameEntry.absoluteFilePath = query.value(3).toString();
      gameEntry.gameDescription = query.value(4).toString();
      gameEntry.systemFullName = query.value(5).toString();
      gameEntry.userSetCore = query.value(6).toString();
      gameEntry.sha1Checksum = query.value(7).toString();

      SystemCoreMap systemCoreMap;
      systemCoreMap.systemFullName = query.value(8).toString();
      systemCoreMap.coreName = query.value(9).toString();
      systemCoreMap.isDefault = query.value(10).toInt() == 0 ? false : true;

      result.append(QPair<GameEntry, SystemCoreMap>(gameEntry, systemCoreMap));
    }
  }

  return result;
}

QStringList LibraryDb::getTableNames() const
{
  return tableNames;
}

void LibraryDb::insert(GameEntry entry)
{
  QMutexLocker locker(dbMutex);

  QSqlDatabase db = databaseConnection();
  QSqlQuery query(db);

  const QString table = "games";

  QVariantHash hash = entry.getQueryFriendlyHash();

  QStringList placeholders;

  for (int i = 0; i < hash.size(); ++i) {
    placeholders.append("?");
  }

  query.prepare(
    QString("INSERT OR IGNORE INTO %1 (%2) VALUES (%3)").arg(
      table
      , hash.keys().join(',')
      , placeholders.join(',')));

  for (QVariant &value : hash) {
    query.addBindValue(value);
  }

  if (!query.exec()) {
    qDebug() << query.lastQuery() << query.lastError().text();
  }
}

void LibraryDb::insert(SystemCoreMap systemCoreMap)
{
  QMutexLocker locker(dbMutex);

  QSqlDatabase db = databaseConnection();
  QSqlQuery query(db);

  const QString table = "system_to_core_mapping";

  QVariantHash hash = systemCoreMap.getQueryFriendlyHash();

  QStringList placeholders;

  for (int i = 0; i < hash.size(); ++i) {
    placeholders.append("?");
  }

  query.prepare(
    QString("INSERT OR IGNORE INTO %1 (%2) VALUES (%3)").arg(
      table
      , hash.keys().join(',')
      , placeholders.join(',')));

  for (QVariant &value : hash) {
    query.addBindValue(value);
  }

  if (!query.exec()) {
    qDebug() << query.lastQuery() << query.lastError().text();
  }
}

void LibraryDb::removeBySha1(QString sha1)
{
  QMutexLocker locker(dbMutex);

  QSqlDatabase db = databaseConnection();
  QSqlQuery query(db);

  query.prepare("DELETE FROM games WHERE sha1Checksum = :sha1");
  query.bindValue(":sha1", sha1);

  if (!(query.exec())) {
    throw std::runtime_error(qPrintable("SQL Error: " + query.lastError().text()));
  }

}

void LibraryDb::removeAllGameEntries()
{
  QMutexLocker locker(dbMutex);

  QSqlDatabase db = databaseConnection();
  QSqlQuery query(db);

  db.transaction();

  if (!query.exec("DELETE FROM games")) {
    throw std::runtime_error(qPrintable(query.lastError().text()));
  }

  db.commit();
}

void LibraryDb::dropAllTables()
{
  QMutexLocker locker(dbMutex);

  QSqlDatabase db = databaseConnection();
  QSqlQuery query(db);

  for (const QString &table : tableNames) {
    if (!query.exec(QString("DROP TABLE IF EXISTS %1").arg(table))) {
      qDebug() << query.lastError().text();
      throw std::runtime_error(qPrintable(QString("Could not drop tables from database %1").arg(
                                            getTableNames().join(", "))));
    }
  }
}

bool LibraryDb::createSchema(QSqlDatabase &db)
{
  QMutexLocker locker(dbMutex);

  db.transaction();

  QSqlQuery query(db);

  if (query.exec("CREATE TABLE schema_version (version INTEGER NOT NULL)")) {
    query.exec(QStringLiteral("INSERT INTO schema_version (version) VALUES (0)"));
  }

  if (query.exec(QStringLiteral("CREATE TABLE games (\n")
                 % QStringLiteral("   rowIndex INTEGER PRIMARY KEY AUTOINCREMENT,\n")


                 % QStringLiteral("   \n/* file info */\n")
                 % QStringLiteral("   absoluteFilePath TEXT UNIQUE NOT NULL,\n")
                 % QStringLiteral("   sha1Checksum TEXT NOT NULL,\n")

                 % QStringLiteral("   \n/* game info */\n")
                 % QStringLiteral("   timePlayed DATETIME,\n")
                 % QStringLiteral("   gameTitle TEXT,\n")
                 % QStringLiteral("   gameImageSource TEXT,\n")
                 % QStringLiteral("   gameDescription TEXT,\n")
                 % QStringLiteral("   systemFullName TEXT,\n")

                 % QStringLiteral("   userSetCore TEXT\n")
                 % QStringLiteral(")"))) {

    query.exec(QStringLiteral("CREATE INDEX title_index ON games (title)"));
    query.exec(QStringLiteral("CREATE INDEX favorite_index ON games (is_favorite)"));
  }

  // Create Collections Mapping Table
  if (query.exec(QStringLiteral("CREATE TABLE collections (\n")
                 % QStringLiteral(" collectionID INTEGER PRIMARY KEY AUTOINCREMENT,\n")
                 % QStringLiteral(" collectionName TEXT UNIQUE NOT NULL\n")
                 % QStringLiteral(")"))) {

    query.exec(QStringLiteral("INSERT INTO collections ")
               % QStringLiteral("(collectionID, collectionName) VALUES (0, 'All')"));
  }

  // Create Collections Table
  query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS collectionMappings (\n")
             % QStringLiteral(" collectionID INTEGER,\n")
             % QStringLiteral(" rowIndex INTEGER,\n")
             % QStringLiteral(" FOREIGN KEY (collectionID) REFERENCES collections ")
             % QStringLiteral("(collectionID) ON DELETE CASCADE ON UPDATE CASCADE\n")
             % QStringLiteral(" FOREIGN KEY (rowIndex) REFERENCES games ")
             % QStringLiteral("(rowIndex) ON DELETE CASCADE ON UPDATE CASCADE\n")
             % QStringLiteral(")"));


  // Create default core table
  if (query.exec(QStringLiteral("CREATE TABLE system_to_core_mapping (\n")
                 % QStringLiteral(" coreName TEXT,")
                 % QStringLiteral(" systemFullName TEXT NOT NULL,")
                 % QStringLiteral(" isDefault INTEGER NOT NULL")
                 % QStringLiteral(")"))) {

//    QList<QPair<QString, QStringList>List >> systemShortNames = {
//      QPair<QString, QStringList>("stella_libretro", { "2600" }),
////      QPair<QString, QStringList>("", {"32X"}),
//      QPair<QString, QStringList>("4do_libretro", {"3DO"}),
//      QPair<QString, QStringList>("atari800_libretro", {"5200"}),
//      QPair<QString, QStringList>("prosystem_libretro", {"7800"}),
//      QPair<QString, QStringList>("ColecoVision", {"bluemsx_libretro"}),
////      QPair<QString, QStringList>("", {"FDS"}),
////      QPair<QString, QStringList>("", {"GB"}),
////      QPair<QString, QStringList>("", {"GBA"}),
////      QPair<QString, QStringList>("", {"GBC"}),
////      QPair<QString, QStringList>("", {"GG"}),
////      QPair<QString, QStringList>("", {"Intellivision"}),
//      QPair<QString, QStringList>("virtualjaguar_libretro", {"Jaguar"}),
////      QPair<QString, QStringList>("", {"Jaguar CD"}),
//      QPair<QString, QStringList>("handy_libretro", {"Lynx"}),
//      QPair<QString, QStringList>("mame_libretro", {"MAME"}),
////      QPair<QString, QStringList>("", {"MD"}),
//      QPair<QString, QStringList>("mupen64plus_libretro", {"N64"}),
////      QPair<QString, QStringList>("", {"NDS"}),
//      QPair<QString, QStringList>("", {"NES"}),
////      QPair<QString, QStringList>("", {"NGC"}),
////      QPair<QString, QStringList>("", {"NGP"}),
////      QPair<QString, QStringList>("", {"NGPC"}),
//      QPair<QString, QStringList>("o2em_libretro", {"Odyssey2"}),
//      QPair<QString, QStringList>("mednafen_pce_fast_libretro", {"PCE"}),
////      QPair<QString, QStringList>("", {"PCECD"}),
//      QPair<QString, QStringList>("mednafen_pcfx_libretro", {"PCFX"}),
////      QPair<QString, QStringList>("", {"PSP"}),
//      QPair<QString, QStringList>("mednafen_psx_libretro", {"PSX"}),
////      QPair<QString, QStringList>("", {"SCD"}),
////      QPair<QString, QStringList>("", {"SG1000"}),
////      QPair<QString, QStringList>("", {"SMS"}),
//      QPair<QString, QStringList>("", {"SNES"}),
//      QPair<QString, QStringList>("yabause_libretro", {"Saturn"}),
////      QPair<QString, QStringList>("", {"SuperGrafx"}),
////      QPair<QString, QStringList>("", {"VB"}),
//      QPair<QString, QStringList>("vecx_libretro", {"Vectrex"}),
////      QPair<QString, QStringList>("", {"Wii"}),
//      QPair<QString, QStringList>("mednafen_wswan_libretro", {"WonderSwan", "WonderSwan Color"}),
//    };

//    query.exec(QStringLiteral("INSERT INTO cores ")
//               % QStringLiteral("(coreName, coreAbsoluteFilePath) VALUES (0, 'All')"));
  }

  return db.commit();
}
