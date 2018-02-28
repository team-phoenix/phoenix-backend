#include "doctest.hpp"
#include "gamemetadata.h"

SCENARIO("GameMetadata")
{
  GIVEN("a real subject") {

    GameMetadata subject;

    WHEN("the GameEntry / Release constructor is called") {

      GameEntry entry;

      entry.absoluteFilePath = "/path/";
      entry.sha1Checksum = "checksum";
      entry.gameImageSource = "frontCover.img";
      entry.gameDescription = "some description";
      entry.gameTitle = "some title";
      entry.systemFullName = "some system";
      entry.userSetCore = "some core";

      SystemCoreMap systemCoreMap;
      systemCoreMap.coreName = "some core name";
      systemCoreMap.systemFullName = "some system full name";

      subject = GameMetadata(entry, systemCoreMap);

      THEN("the member variables are read and assigned correctly") {
        REQUIRE(subject.gameFilePath == entry.absoluteFilePath);
        REQUIRE(subject.gameSha1Checksum == entry.sha1Checksum);
        REQUIRE(subject.gameImageSource == entry.gameImageSource);
        REQUIRE(subject.gameDescription == entry.gameDescription);
        REQUIRE(subject.gameTitle == entry.gameTitle);
        REQUIRE(subject.systemName == entry.systemFullName);
        REQUIRE(subject.userSetCore == entry.userSetCore);

        REQUIRE(subject.defaultCore == systemCoreMap.coreName);

      }
    }
  }
}
