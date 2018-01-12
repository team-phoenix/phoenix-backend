#include "openvgdb.h"
#include "strutils.h"
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

QList<System> OpenVgDb::findAllSystems()
{
  return findAllBy<System>("SYSTEMS", "*");
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

QList<QList<Release> > OpenVgDb::findBatchReleasesBySha1List(QVariantList sha1List)
{
  QSqlDatabase db = databaseConnection();
  QSqlQuery query = QSqlQuery(db);

  query.prepare("SELECT RELEASES.* FROM RELEASES INNER JOIN ROMs "
                "ON ROMs.romID = RELEASES.romID WHERE ROMS.romHashSHA1 = ?;");

  query.addBindValue(sha1List);

  QList<QList<Release>> result;

  if (query.execBatch()) {

    qDebug() << "exec";

    while (query.next()) {
      qDebug() << "batch record size: " << query.size();
//      result.append(Release(getHashedRowValues(db.record("RELEASES"), query)));
    }
  } else {
    qDebug() << query.lastError().text() << query.lastQuery();
  }

  return result;
}

QList<QPair<Release, System>> OpenVgDb::findReleasesByTitle(QString title)
{
  QSqlDatabase db = databaseConnection();
  QSqlQuery query = QSqlQuery(db);

  query.prepare("SELECT SYSTEMS.systemName, RELEASES.releaseCoverFront, RELEASES.releaseDescription, RELEASES.releaseTitleName "
                "FROM RELEASES "
                "INNER JOIN ROMs ON ROMs.romID = RELEASES.romID "
                "INNER JOIN SYSTEMS ON SYSTEMS.systemID = ROMs.systemID "
                "WHERE LOWER(ROMs.romExtensionlessFileName) LIKE LOWER(:cleanedTitle)");

  QString cleanedTitle = StrUtils::normalizePathStr(title);
  cleanedTitle.replace(" ", "%").prepend("%").append("%");
  query.bindValue(":cleanedTitle", cleanedTitle);

  QList<QPair<Release, System>> result;

  if (query.exec()) {
    while (query.next()) {

      System system;
      system.systemName = query.value(0).toString();

      Release release;
      release.releaseCoverFront = query.value(1).toString();
      release.releaseDescription = query.value(2).toString();
      release.releaseTitleName = query.value(3).toString();

      result.append(QPair<Release, System>(release, system));
    }
  } else {
    qDebug() << query.lastError().text() << query.lastQuery();
  }

  return result;
}

QPair<Release, System> OpenVgDb::findReleasesByTitleWithBestGuess(QString title)
{
  QList<QPair<Release, System>> pairs = findReleasesByTitle(title);

  QPair<Release, System> bestMatchPair;

  if (!pairs.isEmpty()) {
    QStringList titleMatches;

    for (const QPair<Release, System> releaseSystemPair : pairs) {
      const Release release = releaseSystemPair.first;
      titleMatches.append(release.releaseTitleName);
    }

    const QString bestMatchTitle = StrUtils::findClosestMatch(titleMatches, title);


    for (const QPair<Release, System> releaseSystemPair : pairs) {
      const Release release = releaseSystemPair.first;

      if (release.releaseTitleName == bestMatchTitle) {
        bestMatchPair = releaseSystemPair;
        break;
      }
    }
  }

  return bestMatchPair;
}
