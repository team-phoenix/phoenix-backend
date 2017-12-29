#include "database.h"

#include <QSqlQuery>

Database::Database(QString databaseFilePath)
{
  this->databaseFilePath = databaseFilePath;
}

QSqlDatabase Database::databaseConnection()
{
  QSqlDatabase database;

  if (!QSqlDatabase::contains(databaseFilePath)) {
    database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseFilePath);
    database.setDatabaseName(databaseFilePath);
  } else {
    database = QSqlDatabase::database(databaseFilePath);
  }

  if (!database.open()) {
    throw std::runtime_error(qPrintable(database.connectionName() + "could not be opened"));
  }

  return database;
}

QVariantHash Database::getHashedRowValues(const QSqlRecord &record, const QSqlQuery &query)
{
  QVariantHash hash;

  for (int i = 0; i < record.count(); ++i) {
    QSqlField field = record.field(i);
    hash.insert(field.name(), query.value(field.name()));
  }

  return hash;
}
