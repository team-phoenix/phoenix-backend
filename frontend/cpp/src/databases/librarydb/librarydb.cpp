
#include "librarydb.h"
#include "logging.h"

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
  tableNames = QStringList({ "games", "collections", "collectionMappings", "defaultCores"});
  createSchema(db);
}

QList<GameEntry> LibraryDb::findAllByGameEntry()
{
  QMutexLocker locker(dbMutex);
  return findAllBy<GameEntry>("games", "*");
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

                 % QStringLiteral("   userSetCore INTEGER,\n")
                 % QStringLiteral("   defaultCore INTEGER\n")

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
  query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS defaultCores (\n")
             % QStringLiteral(" system TEXT UNIQUE NOT NULL,")
             % QStringLiteral(" defaultCore TEXT")
             % QStringLiteral(")"));

  return db.commit();
}
