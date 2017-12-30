#pragma once

#include "gameentry.h"
#include "release.h"

#include <QMetaType>

struct GameMetadata {
  QString gameFilePath;
  QString coreFilePath;

  QString gameTitle;
  QString coreName;

  QString gameImageSource;
  QString gameDescription;

  GameMetadata(GameEntry gameEntry, Release release)
  {
    gameFilePath = gameEntry.absoluteFilePath;
    gameImageSource = release.releaseCoverFront;
    gameDescription = release.releaseDescription;
    gameTitle = release.releaseTitleName;
  }

  GameMetadata() = default;

};

Q_DECLARE_METATYPE(GameMetadata)
