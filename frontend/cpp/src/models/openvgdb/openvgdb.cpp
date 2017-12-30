#include "openvgdb.h"

#include <QCoreApplication>

OpenVgDb::OpenVgDb()
  : Database(QCoreApplication::applicationDirPath() +
             QString("/databases/openvgdb.sqlite"))
{

}

QList<Release> OpenVgDb::findAllReleases()
{
  return findAllBy<Release>("RELEASES", "*");
}

QList<QVariantHash> OpenVgDb::findAllRegions()
{
  return findAllBy<QVariantHash>("REGIONS", "*");
}

QList<QVariantHash> OpenVgDb::findAllRoms()
{
  return findAllBy<QVariantHash>("ROMs", "*");
}

QList<QVariantHash> OpenVgDb::findAllSystems()
{
  return findAllBy<QVariantHash>("SYSTEMS", "*");
}

QList<Release> OpenVgDb::findReleasesByRomID(QVariant romID)
{
  return findRowsByAndWhere<Release>("RELEASES", "romID", romID);
}

QList<Rom> OpenVgDb::findRomsBySha1(QVariant sha1)
{
  return findRowsByAndWhere<Rom>("ROMs", "romHashSHA1", sha1);
}

QList<Release> OpenVgDb::findReleasesBySha1(QVariant sha1)
{
  QSqlDatabase db = databaseConnection();
  QSqlQuery query = QSqlQuery(db);

  query.prepare("SELECT RELEASES.* FROM RELEASES INNER JOIN ROMs "
                "ON ROMs.romID = RELEASES.romID WHERE ROMS.romHashSHA1 = ?;");
  query.addBindValue(sha1);

  QList<Release> result;

  if (query.exec()) {
    while (query.next()) {
      result.append(Release(getHashedRowValues(db.record("RELEASES"), query)));
    }
  } else {
    qDebug() << query.lastError().text() << query.lastQuery();
  }

  return result;
}
