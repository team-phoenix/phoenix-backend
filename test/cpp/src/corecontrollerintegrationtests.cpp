#include "catch.hpp"

#include "corecontroller.hpp"

#include <QDir>
#include <QFile>

SCENARIO("core controller can handle loading games and cores with real dependencies")
{
  GIVEN("a real subject") {

    CoreController subject;

    const QString qrcCorePath = ":/snes9x_libretro.dll";
    const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

    const QString workingCorePath = QDir::temp().filePath("tempCore.sfc");
    const QString workingGamePath = QDir::temp().filePath("tempGame");

    QFile::copy(qrcCorePath, workingCorePath);
    QFile::copy(qrcGamePath, workingGamePath);

    REQUIRE(QFile::exists(workingCorePath) == true);
    REQUIRE(QFile::exists(workingGamePath) == true);

    WHEN("init() is called with a valid core and game") {
      REQUIRE(subject.init(workingCorePath, workingGamePath).isEmpty == false);
      subject.run();
    }

    WHEN("init() is called with an invalid core and game") {
      REQUIRE(subject.init("bad/core/path", "bad/game/path").isEmpty == true);
    }

    WHEN("fini() is called after a successful init()") {
      REQUIRE(subject.init(workingCorePath, workingGamePath).isEmpty == false);
      subject.fini();
    }
  }
}
