//#include "doctest.hpp"

//#include "gameimporter.h"
//#include "tempdbsession.h"

//#include <QCoreApplication>
//#include <QSignalSpy>

//static void insertSystemIntoMap(LibraryDb &libraryDb)
//{
//  SystemCoreMap systemCoreMap;
//  systemCoreMap.systemFullName = "Nintendo Entertainment System";
//  systemCoreMap.coreName = "bnes_libretro";
//  systemCoreMap.isDefault = 1;

//  libraryDb.insert(systemCoreMap);
//}

//SCENARIO("GameImporterIntegrationTests")
//{
//  GIVEN("a real game importer") {

//    const QString testFilePath = QCoreApplication::applicationDirPath() +
//                                 "/testFiles/89 Dennou Kyuusei Uranai.nes";

//    const QString reallyBadPath = "kkakglaklgkla/hahklaklh";

//    GameImporter gameImporter;

//    QSignalSpy spyUpdateModel(&gameImporter, &GameImporter::updateModel);

//    WHEN("a real game is asked to be imported") {

//      gameImporter.importGames(QList<QUrl>({
//        QUrl("file:///" + testFilePath)
//      }));

//      spyUpdateModel.wait();

//      THEN() {
//        REQUIRE(spyUpdateModel.count() == 1);

//        LibraryDb libraryDb;
//        TempDbSession tempSession(&libraryDb);
//        insertSystemIntoMap(libraryDb);

//        QList<QPair<GameEntry, SystemCoreMap>> gameEntries = libraryDb.findAllByGameEntry();
//        REQUIRE(gameEntries.size() == 1);

//        const QPair<GameEntry, SystemCoreMap> &entryPair = gameEntries.first();
//        const GameEntry entry = entryPair.first;

//        REQUIRE(entry.absoluteFilePath == testFilePath);
//        REQUIRE(entry.sha1Checksum == "356A192B7913B04C54574D18C28D46E6395428AB");
//        REQUIRE(entry.userSetCore.isEmpty() == true);
//        REQUIRE(entry.timePlayed.isValid() == false);
//        REQUIRE(entry.gameImageSource == "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg");
//        REQUIRE(entry.gameDescription ==
//                "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988.");
//        REQUIRE(entry.gameTitle == "'89 Dennou Kyuusei Uranai");
//        REQUIRE(entry.systemFullName == "Nintendo Entertainment System");
//      }
//    }
//  }
//}
