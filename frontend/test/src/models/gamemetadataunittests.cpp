#include "catch.hpp"
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

      subject = GameMetadata(entry);

      THEN("the member variables are read and assigned correctly") {
        REQUIRE(subject.gameFilePath == entry.absoluteFilePath);
        REQUIRE(subject.gameSha1Checksum == entry.sha1Checksum);
        REQUIRE(subject.gameImageSource == entry.gameImageSource);
        REQUIRE(subject.gameDescription == entry.gameDescription);
        REQUIRE(subject.gameTitle == entry.gameTitle);
        REQUIRE(subject.systemName == entry.systemFullName);
      }
    }
  }
}
