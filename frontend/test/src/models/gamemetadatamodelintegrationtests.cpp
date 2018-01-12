#include "catch.hpp"
#include "gamemetadatamodel.h"
#include "librarydb.h"
#include "tempdbsession.h"

void insertTestGamesIntoDb(LibraryDb &libraryDb)
{
  libraryDb.insert(GameEntry(QVariantHash({
    { "absoluteFilePath", "/path/to/'89 Dennou Kyuusei Uranai" },
    { "sha1Checksum", "56FE858D1035DCE4B68520F457A0858BAE7BB16D" },
    { "gameTitle", "'89 Dennou Kyuusei Uranai" },
    { "gameImageSource", "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg" },
    { "gameDescription", "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988." },
    { "systemFullName", "Nintendo Entertainment System" },
  })));

  libraryDb.insert(GameEntry(QVariantHash({
    { "absoluteFilePath", "/path/to/4 in 1 Row" },
    { "sha1Checksum", "E9A8996C4FB87120D8620AB8876B90DDB48335DB" },
    { "gameTitle", "4 in 1 Row" },
    { "gameImageSource", "http://img.gamefaqs.net/box/8/1/7/162817_front.jpg" },
    { "gameDescription", "4 in 1 Row is a Puzzle game, developed and published by Philips, which was released in Europe in 1982." },
    { "systemFullName", "Magnavox Odyssey2" },
  })));
}

SCENARIO("GameMetadataModel")
{
  GIVEN("a real subject") {
    GameMetadataModel &subject = GameMetadataModel::instance();

    WHEN("the constructor is called") {
      THEN("the member variables are filled in with items") {
        REQUIRE(subject.rowCount() == 0);
        REQUIRE(subject.columnCount() == 4);
      }
    }

    WHEN("forceUpdate(), is called") {

      LibraryDb libraryDb;
      TempDbSession tempDbSession(&libraryDb);

      insertTestGamesIntoDb(libraryDb);
      subject.forceUpdate();

      THEN("The test rows were set and can be retrieved") {
        REQUIRE(subject.rowCount() == 2);

        const int columnDoesntMatter = -1;
        const QModelIndex firstIndex = subject.createIndexAt(0, columnDoesntMatter);

        const  QString c = subject.data(firstIndex,
                                        GameMetadataModel::Title).toString();
        REQUIRE(c == "'89 Dennou Kyuusei Uranai");
        REQUIRE(subject.data(firstIndex,
                             GameMetadataModel::System).toString() == "Nintendo Entertainment System");
        REQUIRE(subject.data(firstIndex,
                             GameMetadataModel::Description).toString() ==
                "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988.");
        REQUIRE(subject.data(firstIndex,
                             GameMetadataModel::ImageSource).toString() == "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg");

        const QModelIndex secondIndex = subject.createIndexAt(1, columnDoesntMatter);

        REQUIRE(subject.data(secondIndex, GameMetadataModel::Title).toString() == "4 in 1 Row");
        REQUIRE(subject.data(secondIndex, GameMetadataModel::System).toString() == "Magnavox Odyssey2");
        REQUIRE(subject.data(secondIndex,
                             GameMetadataModel::Description).toString() ==
                "4 in 1 Row is a Puzzle game, developed and published by Philips, which was released in Europe in 1982.");
        REQUIRE(subject.data(secondIndex,
                             GameMetadataModel::ImageSource).toString() == "http://img.gamefaqs.net/box/8/1/7/162817_front.jpg");
      }

      WHEN("the database is cleared") {
        libraryDb.removeAllGameEntries();

        THEN("a second forceUpdate() produces an empty rowCount()") {
          subject.forceUpdate();
          REQUIRE(subject.rowCount() == 0);
        }
      }

      WHEN("the database is reupdated") {
        LibraryDb libraryDb;
        TempDbSession tempDbSession(&libraryDb);
        insertTestGamesIntoDb(libraryDb);

        THEN("a final forceUpdate() produces an non empty rowCount()") {
          subject.forceUpdate();
          REQUIRE(subject.rowCount() == 2);
        }
      }
    }

    WHEN("a remove game by sha1 is called") {
      LibraryDb libraryDb;
      TempDbSession tempDbSession(&libraryDb);
      insertTestGamesIntoDb(libraryDb);

      subject.forceUpdate();

      REQUIRE(subject.rowCount() == 2);

      THEN("the games cache is reduced by one") {
        subject.removeGameAt(0);
        subject.forceUpdate();
        REQUIRE(subject.rowCount() == 1);
      }
    }
  }
}
