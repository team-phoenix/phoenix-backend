#pragma once

#include "gameentry.h"
#include "release.h"
#include "systemcoremap.h"

#include <QMetaType>

struct GameMetadata {
  QString gameFilePath;
  QString coreFilePath;

  QString gameTitle;
  QString systemName;
  QString defaultCore;
  QString userSetCore;

  QString gameImageSource;
  QString gameDescription;
  QString gameSha1Checksum;

  GameMetadata(GameEntry gameEntry, SystemCoreMap systemCoreMap)
  {
    gameFilePath = gameEntry.absoluteFilePath;
    gameSha1Checksum = gameEntry.sha1Checksum;
    gameImageSource = gameEntry.gameImageSource;
    gameDescription = gameEntry.gameDescription;
    gameTitle = gameEntry.gameTitle;
    systemName = gameEntry.systemFullName;
    defaultCore = systemCoreMap.coreName;
    userSetCore = gameEntry.userSetCore;
  }

  GameMetadata() = default;

};

Q_DECLARE_METATYPE(GameMetadata)
