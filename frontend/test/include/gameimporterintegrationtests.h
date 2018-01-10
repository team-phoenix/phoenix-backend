#include "gameimporter.h"

#include <QTest>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QCoreApplication>
#include <QSignalSpy>

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
      QUrl(testFilePath)
    }));
    spyUpdateModel.wait();
    QCOMPARE(spyUpdateModel.count(), 1);
  }

//  void a_game_cannot_be_imported_with_an_invalid_path()
//  {
//    QSignalSpy spyUpdateModel(gameImporter, &GameImporter::updateModel);
//    gameImporter->importGames(QList<QString>({
//      QString(reallyBadPath),
//      "kalkjlfaklgfklaklga"
//    }));
//    spyUpdateModel.wait();
//    QCOMPARE(spyUpdateModel.count(), 0);
//  }
};
