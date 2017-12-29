#pragma once

#include <QSqlDatabase>

#include <QStringBuilder>
#include <QCoreApplication>
#include <QVariantList>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>
#include <QSqlError>

struct Release {
  int releaseID{ -1 };
  int romID{ -1 };
  QString releaseTitleName;
  int regionLocalizedID{ -1 };
  QString TEMPregionLocalizedName;
  QString TEMPsystemShortName;
  QString TEMPsystemName;
  QString releaseCoverFront;
  QString releaseCoverBack;
  QString releaseCoverCart;
  QString releaseCoverDisc;
  QString releaseDescription;
  QString releaseDeveloper;
  QString releasePublisher;
  QString releaseGenre;
  QString releaseDate;
  QString releaseReferenceURL;
  QString releaseReferenceImageURL;

  Release(const QVariantHash &hash)
  {
    releaseID = hash.value("releaseID").toInt();
    romID = hash.value("romID").toInt();
    releaseTitleName = hash.value("releaseTitleName").toString();
    releaseID = hash.value("releaseID").toInt();
    TEMPregionLocalizedName = hash.value("TEMPregionLocalizedName").toString();
    TEMPsystemShortName = hash.value("TEMPsystemShortName").toString();
    TEMPsystemName = hash.value("TEMPsystemName").toString();
    releaseCoverFront = hash.value("releaseCoverFront").toString();
    releaseCoverBack = hash.value("releaseCoverBack").toString();
    releaseCoverCart = hash.value("releaseCoverCart").toString();
    releaseCoverDisc = hash.value("releaseCoverDisc").toString();
    releaseDescription = hash.value("releaseDescription").toString();
    releaseDeveloper = hash.value("releaseDeveloper").toString();
    releasePublisher = hash.value("releasePublisher").toString();
    releaseGenre = hash.value("releaseGenre").toString();
    releaseDate = hash.value("releaseDate").toString();
    releaseReferenceURL = hash.value("releaseReferenceURL").toString();
    releaseReferenceImageURL = hash.value("releaseReferenceImageURL").toString();
  }
};

struct Rom {
  int romID{ -1 };
  int systemID{ -1 };
  int regionID{ -1 };
  QString romHashCRC;
  QString romHashMD5;
  QString romHashSHA1;
  int romSize{ -1 };
  QString romFileName;
  QString romExtensionlessFileName;
  QString romSerial;
  QString romHeader;
  QString romLanguage;
  QString TEMPromRegion;
  QString romDumpSource;

  Rom(const QVariantHash &hash)
  {
    romID = hash.value("romID").toInt();
    systemID = hash.value("systemID").toInt();
    regionID = hash.value("regionID").toInt();
    romHashCRC = hash.value("romHashCRC").toString();
    romHashMD5 = hash.value("romHashMD5").toString();
    romHashSHA1 = hash.value("romHashSHA1").toString();
    romSize = hash.value("romSize").toInt();
    romFileName = hash.value("romFileName").toString();
    romExtensionlessFileName = hash.value("romExtensionlessFileName").toString();
    romSerial = hash.value("romSerial").toString();
    romHeader = hash.value("romHeader").toString();
    romLanguage = hash.value("romLanguage").toString();
    TEMPromRegion = hash.value("TEMPromRegion").toString();
    romDumpSource = hash.value("romDumpSource").toString();
  }
};

class Database
{
public:
  Database(QString databaseFilePath)
  {
    this->databaseFilePath = databaseFilePath;
  }

protected:
  QSqlDatabase databaseConnection()
  {
    QSqlDatabase database;

    QString connectionName = QStringLiteral("metadata");

    if (!QSqlDatabase::contains(connectionName)) {
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

  QList<QVariantHash> findAllBy(QString table, QString rowName)
  {
    QSqlDatabase db = databaseConnection();
    QSqlQuery query = QSqlQuery(db);

    QList<QVariantHash> result;

    if (query.exec(QString("SELECT %1 from %2").arg(rowName, table))) {
      while (query.next()) {
        QSqlRecord record = db.record(table);

        QVariantHash hash;

        for (int i = 0; i < record.count(); ++i) {
          QSqlField field = record.field(i);
          hash.insert(field.name(), query.value(field.name()));
        }

        result.append(hash);
      }
    }

    return result;
  }

private:
  QString databaseFilePath;
};

class GameMetadataDB : public Database
{
public:
  GameMetadataDB();

  QList<QVariantHash> findAllReleases();

  QList<QVariantHash> findAllRegions();

  QList<QVariantHash> findAllRoms();

  QList<QVariantHash> findAllSystems();

  template<typename Result>
  QList<Result> findRowsBy(QString table, QString row, QVariant value)
  {
    QSqlDatabase db = databaseConnection();
    QSqlQuery query = QSqlQuery(db);

    query.prepare(QString("SELECT * from %1 WHERE %2 = ?").arg(table, row));
    query.addBindValue(value);

    QList<Result> result;

    if (query.exec()) {
      while (query.next()) {
        QSqlRecord record = db.record(table);

        QVariantHash hash;

        for (int i = 0; i < record.count(); ++i) {
          QSqlField field = record.field(i);
          hash.insert(field.name(), query.value(field.name()));
        }

        result.append(Result(hash));
      }
    } else {
      qDebug() << query.lastError().text() << query.lastQuery();
    }

    return result;
  }

  QList<Release> findReleasesByRomID(QVariant romID)
  {
    return findRowsBy<Release>("RELEASES", "romID", romID);
  }

  QList<Rom> findRomsBySha1(QVariant sha1)
  {
    return findRowsBy<Rom>("ROMs", "romHashSHA1", sha1);
  }
};
