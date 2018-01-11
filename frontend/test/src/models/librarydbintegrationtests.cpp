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
      })));

      THEN("the database will contain the game entry uniquely") {
        auto entries = subject.findAllByGameEntry();
        REQUIRE(entries.size() == 1);
        REQUIRE(entries.first().absoluteFilePath == "1234");
        REQUIRE(entries.first().sha1Checksum == "goodstuff");
        REQUIRE(entries.first().gameImageSource == "/path/to/image");
        REQUIRE(entries.first().gameDescription == "some description");
      }

      THEN("the database can be cleared") {
        subject.removeAllGameEntries();
        auto entries = subject.findAllByGameEntry();
        REQUIRE(entries.isEmpty() == true);
      }
    }
  }
}
