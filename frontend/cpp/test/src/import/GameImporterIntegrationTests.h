#pragma once

#include "gameimporter.h"
#include "TempDbSession.h"

#include <QTest>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QCoreApplication>
#include <QSignalSpy>

#include <QSqlDatabase>
#include <QSqlQuery>

static void insertSystemIntoMap(LibraryDb &libraryDb)
{
  SystemCoreMap systemCoreMap;
  systemCoreMap.systemFullName = "Nintendo Entertainment System";
  systemCoreMap.coreName = "bnes_libretro";
  systemCoreMap.isDefault = 1;

  libraryDb.insert(systemCoreMap);
}

class GameImporterIntegrationTests : public QObject
{
  Q_OBJECT
  GameImporter* gameImporter;

  const QString testFilePath = QCoreApplication::applicationDirPath() +
                               "/testFiles/89 Dennou Kyuusei Uranai.nes";

  const QString reallyBadPath = "kkakglaklgkla/hahklaklh";
public:
  GameImporterIntegrationTests() = default;

private slots:

  void initTestCase()
  {
    gameImporter = new GameImporter;
  }

  void cleanupTestCase()
  {
    delete gameImporter;
  }

  void a_game_can_be_imported_with_a_valid_path()
  {
    QSignalSpy spyUpdateModel(gameImporter, &GameImporter::updateModel);
    gameImporter->importGames(QList<QUrl>({
      QUrl("file:///" + testFilePath)
    }));
    spyUpdateModel.wait();
    QCOMPARE(spyUpdateModel.count(), 1);

    LibraryDb libraryDb;
    TempDbSession tempSession(&libraryDb);
    insertSystemIntoMap(libraryDb);

    QList<QPair<GameEntry, SystemCoreMap>> gameEntries = libraryDb.findAllByGameEntry();
    QCOMPARE(gameEntries.size(), 1);

    const QPair<GameEntry, SystemCoreMap> &entryPair = gameEntries.first();
    const GameEntry entry = entryPair.first;

    QCOMPARE(entry.absoluteFilePath, testFilePath);
    QCOMPARE(entry.sha1Checksum, "356A192B7913B04C54574D18C28D46E6395428AB");
    QCOMPARE(entry.userSetCore.isEmpty(), true);
    QCOMPARE(entry.timePlayed.isValid(), false);
    QCOMPARE(entry.gameImageSource, "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg");
    QCOMPARE(entry.gameDescription,
             "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988.");
    QCOMPARE(entry.gameTitle, "'89 Dennou Kyuusei Uranai");
    QCOMPARE(entry.systemFullName, "Nintendo Entertainment System");

  }

  void a_game_cannot_be_imported_with_an_invalid_path()
  {
    QSignalSpy spyUpdateModel(gameImporter, &GameImporter::updateModel);
    gameImporter->importGames(QList<QUrl>({
      QUrl(reallyBadPath),
      QUrl("kalkjlfaklgfklaklga")
    }));
    spyUpdateModel.wait();
    QCOMPARE(spyUpdateModel.count(), 1);

    LibraryDb libraryDb;
    TempDbSession tempSession(&libraryDb);

    QCOMPARE(libraryDb.findAllByGameEntry().size(), 0);
  }
};
