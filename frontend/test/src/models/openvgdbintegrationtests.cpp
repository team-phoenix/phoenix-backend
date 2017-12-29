#include "catch.hpp"
#include "openvgdb.h"

#include <QFile>

SCENARIO("OpenVgDb")
{
  GIVEN("A real OpenVgDb class") {
    OpenVgDb subject;

    WHEN("the constructor is called") {
      THEN("The database exists and is being used") {
        REQUIRE(QFile::exists(subject.filePath()) == true);
      }
    }

    WHEN("findAllReleases(), is called") {
      QList<Release> releases = subject.findAllReleases();
      THEN("It returns a non empty list") {
        REQUIRE(releases.size() == 2);
      }
    }

    WHEN("findAllRegions(), is called") {
      QList<QVariantHash> regions = subject.findAllRegions();
      THEN("It returns a non empty list") {
        REQUIRE(regions.size() == 2);
      }
    }

    WHEN("findAllRoms(), is called") {
      QList<QVariantHash> roms = subject.findAllRoms();
      THEN("It returns a non empty list") {
        REQUIRE(roms.size() == 2);
      }
    }

    WHEN("findAllReleases(), is called") {
      QList<QVariantHash> systems = subject.findAllSystems();
      THEN("It returns a non empty list") {
        REQUIRE(systems.size() == 2);
      }
    }
  }
}
