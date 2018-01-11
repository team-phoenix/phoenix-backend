#include "catch.hpp"
#include "gameentry.h"

SCENARIO("GameEntry")
{
  GIVEN("a real subject") {
    GameEntry subject;

    WHEN("the QVariantHash constructor is called") {
      subject = GameEntry(QVariantHash({
        { "rowIndex", 2 },
        { "absoluteFilePath", "1234" },
        { "sha1Checksum", "goodstuff" },
        {
          "timePlayed", QDateTime::fromString("M1d1y9800:01:02",
                                              "'M'M'd'd'y'yyhh:mm:ss")
        },
        {"gameImageSource", "/path/to/image"},
        {"gameDescription", "some description"},
        { "userSetCore", 3 },
        { "defaultCore", 4 },
      }));

      THEN("the hash is parsed correctly") {
        REQUIRE(subject.rowIndex == 2);
        REQUIRE(subject.absoluteFilePath == "1234");
        REQUIRE(subject.sha1Checksum == "goodstuff");
        REQUIRE(subject.timePlayed == QDateTime::fromString("M1d1y9800:01:02",
                                                            "'M'M'd'd'y'yyhh:mm:ss"));
        REQUIRE(subject.gameImageSource == "/path/to/image");
        REQUIRE(subject.gameDescription == "some description");
        REQUIRE(subject.userSetCore == 3);
        REQUIRE(subject.defaultCore == 4);
      }

      THEN("getting the query friendly hash returns all members except the rowIndex") {
        QVariantHash queryMap = subject.getQueryFriendlyHash();
        REQUIRE(queryMap.size() == 7);
        REQUIRE(queryMap.contains("rowIndex") == false);
        REQUIRE(queryMap["absoluteFilePath"].toString() == "1234");
        REQUIRE(queryMap["sha1Checksum"].toString() == "goodstuff");
        REQUIRE(queryMap["timePlayed"].toDateTime() == QDateTime::fromString("M1d1y9800:01:02",
                                                                             "'M'M'd'd'y'yyhh:mm:ss"));
        REQUIRE(queryMap["gameImageSource"].toString() == "/path/to/image");
        REQUIRE(queryMap["gameDescription"].toString() == "some description");
        REQUIRE(queryMap["userSetCore"].toInt() == 3);
        REQUIRE(queryMap["defaultCore"].toInt() == 4);

      }
    }
  }
}
