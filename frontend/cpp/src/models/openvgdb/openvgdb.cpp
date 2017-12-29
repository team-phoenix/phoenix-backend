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
