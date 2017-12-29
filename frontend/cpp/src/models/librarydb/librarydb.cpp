
#include "librarydb.h"
#include "logging.h"

#include <QCoreApplication>
#include <QStringBuilder>
#include <QSqlQuery>

LibraryDb::LibraryDb()
  : Database(QCoreApplication::applicationDirPath()  + "/databases/librarydb.sqlite")
{
  QSqlDatabase db = databaseConnection();

  if (!db.tables().contains("schema_version")) {
    createSchema(db);
  }
}

QList<GameEntry> LibraryDb::findAllByGameEntry()
{
  return findAllBy<GameEntry>("games", "*");
}

void LibraryDb::insert(GameEntry entry)
{
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

bool LibraryDb::createSchema(QSqlDatabase &db)
{
  qCDebug(phxLibrary, "Initializing Library Schema");
  db.transaction();

  QSqlQuery query(db);
  query.exec("CREATE TABLE schema_version (version INTEGER NOT NULL)");
  query.exec(QStringLiteral("INSERT INTO schema_version (version) VALUES (0)"));
  query.exec(QStringLiteral("CREATE TABLE games (\n")
             % QStringLiteral("   rowIndex INTEGER PRIMARY KEY AUTOINCREMENT,\n")


             % QStringLiteral("   \n/* file info */\n")
             % QStringLiteral("   absoluteFilePath TEXT UNIQUE NOT NULL,\n")
             % QStringLiteral("   sha1Checksum TEXT UNIQUE NOT NULL,\n")

             % QStringLiteral("   \n/* game info */\n")
             % QStringLiteral("   timePlayed DATETIME,\n")

             % QStringLiteral("   userSetCore INTEGER,\n")
             % QStringLiteral("   defaultCore INTEGER\n")

             % QStringLiteral(")"));

  query.exec(QStringLiteral("CREATE INDEX title_index ON games (title)"));
  query.exec(QStringLiteral("CREATE INDEX favorite_index ON games (is_favorite)"));


  // Create Collections Mapping Table
  query.exec(QStringLiteral("CREATE TABLE collections (\n")
             % QStringLiteral(" collectionID INTEGER PRIMARY KEY AUTOINCREMENT,\n")
             % QStringLiteral(" collectionName TEXT UNIQUE NOT NULL\n")
             % QStringLiteral(")"));

  // Create Collections Table
  query.exec(QStringLiteral("CREATE TABLE collectionMappings (\n")
             % QStringLiteral(" collectionID INTEGER,\n")
             % QStringLiteral(" rowIndex INTEGER,\n")
             % QStringLiteral(" FOREIGN KEY (collectionID) REFERENCES collections ")
             % QStringLiteral("(collectionID) ON DELETE CASCADE ON UPDATE CASCADE\n")
             % QStringLiteral(" FOREIGN KEY (rowIndex) REFERENCES games ")
             % QStringLiteral("(rowIndex) ON DELETE CASCADE ON UPDATE CASCADE\n")
             % QStringLiteral(")"));

  query.exec(QStringLiteral("INSERT INTO collections ")
             % QStringLiteral("(collectionID, collectionName) VALUES (0, 'All')"));

  // Create default core table
  query.exec(QStringLiteral("CREATE TABLE defaultCores (\n")
             % QStringLiteral(" system TEXT UNIQUE NOT NULL,")
             % QStringLiteral(" defaultCore TEXT")
             % QStringLiteral(")"));

  return db.commit();

}
