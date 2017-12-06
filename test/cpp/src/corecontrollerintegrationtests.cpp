#include "catch.hpp"

#include "corecontroller.hpp"

#include <QDir>
#include <QFile>

SCENARIO("core controller can handle loading games and cores with real dependencies")
{
  GIVEN("a real subject") {

    CoreController subject;

    const QString qrcCorePath = ":/bsnes_balanced_libretro.dll";
    const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

    const QString workingCorePath = QDir::temp().filePath("tempCore.sfc");
    const QString workingGamePath = QDir::temp().filePath("tempGame");

    QFile::copy(qrcCorePath, workingCorePath);
    QFile::copy(qrcGamePath, workingGamePath);

    REQUIRE(QFile::exists(workingCorePath) == true);
    REQUIRE(QFile::exists(workingGamePath) == true);

    WHEN("init() is called with a valid core and game") {
      REQUIRE(subject.init(workingCorePath, workingGamePath).isEmpty == false);
//      subject.run();
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

SCENARIO("The core environment GET callbacks should be all handled properly")
{
  GIVEN("A real core with callbacks set up") {

    CoreController subject;

    const QString qrcCorePath = ":/bsnes_balanced_libretro.dll";
    const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

    const QString workingCorePath = QDir::temp().filePath("tempCore.sfc");
    const QString workingGamePath = QDir::temp().filePath("tempGame");

    QFile::copy(qrcCorePath, workingCorePath);
    QFile::copy(qrcGamePath, workingGamePath);

    WHEN("init() has been called with a working core and game") {
      REQUIRE(subject.init(workingCorePath, workingGamePath).isEmpty == false);

      THEN("environmentCallback(): RETRO_ENVIRONMENT_GET_OVERSCAN should be set data to true and return true") {
        bool data = false;
        REQUIRE(CoreController::environmentCallback(RETRO_ENVIRONMENT_GET_OVERSCAN, &data));
        REQUIRE(data == true);
      }
// TODO - Finish this

//      THEN("environmentCallback(): RETRO_ENVIRONMENT_GET_VARIABLE should return true and set data to non null") {
//        struct retro_variable variable;
//        variable.key = "snes9x_next_overclock";
//        variable.value = nullptr;
//        REQUIRE(CoreController::environmentCallback(RETRO_ENVIRONMENT_GET_VARIABLE, &variable));
//        REQUIRE(variable.value != nullptr);
//      }
    }
  }
}
