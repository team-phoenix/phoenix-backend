#include "gameimporter.h"
#include "tempdbsession.h"

#include <QTest>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QCoreApplication>
#include <QSignalSpy>

#include <QSqlDatabase>
#include <QSqlQuery>

// TODO - Not working correctly.
class GameImporterIntegrationTests : public QObject
{
  Q_OBJECT
  GameImporter* gameImporter;

  const QString testFilePath = QCoreApplication::applicationDirPath() +
                               "/testFiles/test_rom.nes";

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

    QList<GameEntry> gameEntries = libraryDb.findAllByGameEntry();
    QCOMPARE(gameEntries.size(), 1);

    GameEntry &entry = gameEntries.first();
    QCOMPARE(entry.absoluteFilePath, testFilePath);
    QCOMPARE(entry.sha1Checksum, "356A192B7913B04C54574D18C28D46E6395428AB");
    QCOMPARE(entry.userSetCore, -1);
    QCOMPARE(entry.defaultCore, -1);
    QCOMPARE(entry.timePlayed.isValid(), false);
    QCOMPARE(!entry.gameImageSource.isEmpty(), false);
    QCOMPARE(!entry.gameDescription.isEmpty(), false);
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
