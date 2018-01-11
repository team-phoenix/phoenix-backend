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

      Release release;
      release.releaseCoverFront = "frontCover.img";
      release.releaseDescription = "some description";
      release.releaseTitleName = "some title";
      release.TEMPsystemName = "some system";

      subject = GameMetadata(entry, release);

      THEN("the member variables are read and assigned correctly") {
        REQUIRE(subject.gameFilePath == entry.absoluteFilePath);
        REQUIRE(subject.gameSha1Checksum == entry.sha1Checksum);
        REQUIRE(subject.gameImageSource == entry.gameImageSource);
        REQUIRE(subject.gameDescription == entry.gameDescription);
        REQUIRE(subject.gameTitle == release.releaseTitleName);
        REQUIRE(subject.systemName == release.TEMPsystemName);
      }
    }
  }
}
