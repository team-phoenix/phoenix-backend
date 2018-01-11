#pragma once

#include "gameentry.h"
#include "release.h"

#include <QMetaType>

struct GameMetadata {
  QString gameFilePath;
  QString coreFilePath;

  QString gameTitle;
  QString systemName;
  QString coreName;

  QString gameImageSource;
  QString gameDescription;
  QString gameSha1Checksum;

  GameMetadata(GameEntry gameEntry, Release release)
  {
    gameFilePath = gameEntry.absoluteFilePath;
    gameSha1Checksum = gameEntry.sha1Checksum;
    gameImageSource = gameEntry.gameImageSource;
    gameDescription = gameEntry.gameDescription;
    gameTitle = release.releaseTitleName;
    systemName = release.TEMPsystemName;
  }

  GameMetadata() = default;

};

Q_DECLARE_METATYPE(GameMetadata)
