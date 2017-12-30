#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QSqlQuery>

#include <QVariantHash>
#include <QList>

#include <QDebug>

class Database
{
public:
  Database(QString databaseFilePath);
  QSqlDatabase databaseConnection();

  QString filePath() const;

protected:

  template<typename Result>
  QList<Result> findAllBy(QString table, QString rowName)
  {
    QSqlDatabase db = databaseConnection();
    QSqlQuery query = QSqlQuery(db);

    QList<Result> result;

    if (query.exec(QString("SELECT %1 from %2").arg(rowName, table))) {
      while (query.next()) {
        result.append(Result(getHashedRowValues(db.record(table), query)));
      }
    }

    return result;
  }

  QVariantHash getHashedRowValues(const QSqlRecord &record, const QSqlQuery &query);

  template<typename Result>
  QList<Result> findRowsByAndWhere(QString table, QString row, QVariant value)
  {
    QSqlDatabase db = databaseConnection();
    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("SELECT * from %1 WHERE %2 = ?").arg(table, row));
    query.addBindValue(value);

    QList<Result> result;

    if (query.exec()) {
      while (query.next()) {
        result.append(Result(getHashedRowValues(db.record(table), query)));
      }
    } else {
      qDebug() << query.lastError().text() << query.lastQuery();
    }

    return result;
  }

private:
  QString databaseFilePath;
};
