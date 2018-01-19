#include "catch.hpp"
#include "librarydb.h"
#include "tempdbsession.h"

#include <QFile>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>

SCENARIO("LibraryDb")
{
  GIVEN("A real LibraryDb class") {

    WHEN("the constructor is called") {
      LibraryDb subject;
      TempDbSession tempDbSession(&subject);

      THEN("the database file will have been created") {
        REQUIRE(QFile::exists(subject.filePath()) == true);
      }
    }

    WHEN("a game has been inserted") {
      LibraryDb subject;
      TempDbSession tempDbSession(&subject);

      subject.removeAllGameEntries();

      subject.insert(GameEntry(QVariantHash({
        { "absoluteFilePath", "1234" },
        { "sha1Checksum", "goodstuff" },
        {"gameImageSource", "/path/to/image"},
        {"gameDescription", "some description"},
        {"systemFullName", "some sys name"},
      })));

      SystemCoreMap systemCoreMap;
      systemCoreMap.systemFullName = "some sys name";
      systemCoreMap.isDefault = true;
      systemCoreMap.coreName = "some core name";

      subject.insert(systemCoreMap);

      THEN("the database will contain the game entry uniquely") {

        const QList<QPair<GameEntry, SystemCoreMap>> pairs = subject.findAllByGameEntry();
        REQUIRE(pairs.size() == 1);

        const QPair<GameEntry, SystemCoreMap> &firstPair = pairs.first();
        const GameEntry gameEntry = firstPair.first;

        REQUIRE(gameEntry.absoluteFilePath == "1234");
        REQUIRE(gameEntry.sha1Checksum == "goodstuff");
        REQUIRE(gameEntry.gameImageSource == "/path/to/image");
        REQUIRE(gameEntry.gameDescription == "some description");
      }

      THEN("the database can be cleared") {
        subject.removeAllGameEntries();
        auto entries = subject.findAllByGameEntry();
        REQUIRE(entries.isEmpty() == true);
      }
    }
  }
}
