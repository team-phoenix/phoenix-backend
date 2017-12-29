#pragma once

#include "rom.h"
#include "release.h"
#include "database.h"

class OpenVgDb : public Database
{
public:
  OpenVgDb();

  QList<QVariantHash> findAllReleases();

  QList<QVariantHash> findAllRegions();

  QList<QVariantHash> findAllRoms();

  QList<QVariantHash> findAllSystems();

  QList<Release> findReleasesByRomID(QVariant romID);

  QList<Rom> findRomsBySha1(QVariant sha1);
};
