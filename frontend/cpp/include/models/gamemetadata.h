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

  GameMetadata(GameEntry gameEntry, Release release)
  {
    gameFilePath = gameEntry.absoluteFilePath;
    gameImageSource = release.releaseCoverFront;
    gameDescription = release.releaseDescription;
    gameTitle = release.releaseTitleName;
    systemName = release.TEMPsystemName;
  }

  GameMetadata() = default;

};

Q_DECLARE_METATYPE(GameMetadata)
