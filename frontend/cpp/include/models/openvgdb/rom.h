#pragma once

#include <QVariantHash>
#include <QString>

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
