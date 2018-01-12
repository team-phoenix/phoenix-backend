#pragma once

#include "rom.h"
#include "release.h"
#include "system.h"
#include "database.h"

#include <QPair>

class OpenVgDb : public Database
{
public:
  OpenVgDb();
  ~OpenVgDb() = default;

  QList<Release> findAllReleases();

  QList<QVariantHash> findAllRegions();

  QList<QVariantHash> findAllRoms();

  QList<System> findAllSystems();

  QList<Release> findReleasesByRomID(QVariant romID);

  QList<Rom> findRomsBySha1(QVariant sha1);

  QList<Release> findReleasesBySha1(QVariant sha1);
  QList<QList<Release>> findBatchReleasesBySha1List(QVariantList sha1List);

  QList<QPair<Release, System>> findReleasesByTitle(QString title);

  QPair<Release, System> findReleasesByTitleWithBestGuess(QString title);


};
