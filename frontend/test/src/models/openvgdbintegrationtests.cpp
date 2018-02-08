#include "doctest.hpp"
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
      QList<System> systems = subject.findAllSystems();
      THEN("It returns a non empty list") {
        REQUIRE(systems.size() == 2);
      }
    }

    WHEN("findReleasesByRomID(), is called") {
      QList<Release> releases = subject.findReleasesByRomID(1);
      THEN("It returns a valid release") {
        REQUIRE(releases.size() == 1);

        const Release &release = releases.first();
        REQUIRE(release.releaseID == 57434);
        REQUIRE(release.romID == 1);
        REQUIRE(release.releaseTitleName == "'89 Dennou Kyuusei Uranai");
        REQUIRE(release.regionLocalizedID == 13);
        REQUIRE(release.TEMPregionLocalizedName == "Japan");
        REQUIRE(release.TEMPsystemShortName == "NES");
        REQUIRE(release.TEMPsystemName == "Nintendo Entertainment System");
        REQUIRE(release.releaseCoverFront == "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg");
        REQUIRE(release.releaseCoverBack == "http://img.gamefaqs.net/box/6/4/2/41642_back.jpg");
        REQUIRE(release.releaseCoverCart.isNull() == true);
        REQUIRE(release.releaseCoverDisc.isNull() == true);
        REQUIRE(release.releaseDescription ==
                "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988.");
        REQUIRE(release.releaseDeveloper == "Micronics");
        REQUIRE(release.releasePublisher.isNull() == true);
        REQUIRE(release.releaseGenre == "Miscellaneous,General");
        REQUIRE(release.releaseDate == "Dec 10, 1988");

        REQUIRE(release.releaseReferenceURL ==
                "http://www.gamefaqs.com/nes/579365-89-dennou-kyuusei-uranai");
        REQUIRE(release.releaseReferenceImageURL ==
                "http://www.gamefaqs.com/nes/579365-89-dennou-kyuusei-uranai/images/box-20835");
      }
    }

    WHEN("findReleasesBySha1(), is called with a valid sha1") {
      const QList<Release> releases =
        subject.findReleasesBySha1("56FE858D1035DCE4B68520F457A0858BAE7BB16D");

      THEN("The returned releases will be greater than 0") {
        REQUIRE(releases.size() > 0);
      }

      THEN("The returned release has a romID equal to the ROMs table sha1 row") {
        const Release &release = releases.first();
        REQUIRE(release.romID == 1);
      }
    }

    WHEN("findReleasesByTitle(), is called with a valid title") {
      const QList<QPair<Release, System>> releaseSystemList =
                                         subject.findReleasesByTitle("/path/to/'89 Dennou Kyuusei Uranai (Japan).nes");

      THEN("The returned releases will be 1") {
        REQUIRE(releaseSystemList.size() == 1);
      }

      THEN("The returned release has a romID equal to the ROMs table sha1 row") {
        const QPair<Release, System> &releaseSystemPair = releaseSystemList.first();
        const Release release = releaseSystemPair.first;
        const System system = releaseSystemPair.second;

        REQUIRE(system.systemName == "Nintendo Entertainment System");

        REQUIRE(release.releaseTitleName == "'89 Dennou Kyuusei Uranai");
        REQUIRE(release.releaseCoverFront == "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg");
        REQUIRE(release.releaseDescription ==
                "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988.");

      }
    }

    WHEN("findReleasesByTitleWithBestGuess(), is called with a valid title with a database that contains 3 similar entries") {
      const QPair<Release, System> releaseSystemPair =
        subject.findReleasesByTitleWithBestGuess("/path/to/'89 Dennou Kyuusei Uranai (Japan).nes");

      THEN("The returned release is the one that matches the closest to the game") {
        const Release release = releaseSystemPair.first;
        const System system = releaseSystemPair.second;

        REQUIRE(system.systemName == "Nintendo Entertainment System");

        REQUIRE(release.releaseTitleName == "'89 Dennou Kyuusei Uranai");
        REQUIRE(release.releaseCoverFront == "http://img.gamefaqs.net/box/6/4/2/41642_front.jpg");
        REQUIRE(release.releaseDescription ==
                "'89 Dennou Kyuusei Uranai is a Miscellaneous game, developed by Micronics and published by Jingukan Polaris,which was released in Japan in 1988.");
      }
    }
  }
}
