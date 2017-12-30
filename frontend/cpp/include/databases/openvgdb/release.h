#pragma once

#include <QString>
#include <QVariantHash>

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
    regionLocalizedID = hash.value("regionLocalizedID").toInt();
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

  Release() = default;
};
