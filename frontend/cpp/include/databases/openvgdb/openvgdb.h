#pragma once

#include "rom.h"
#include "release.h"
#include "database.h"

class OpenVgDb : public Database
{
public:
  OpenVgDb();

  QList<Release> findAllReleases();

  QList<QVariantHash> findAllRegions();

  QList<QVariantHash> findAllRoms();

  QList<QVariantHash> findAllSystems();

  QList<Release> findReleasesByRomID(QVariant romID);

  QList<Rom> findRomsBySha1(QVariant sha1);

  QList<Release> findReleasesBySha1(QVariant sha1);
};
