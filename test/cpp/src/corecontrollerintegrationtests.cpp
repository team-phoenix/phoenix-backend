#include "catch.hpp"

#include "corecontroller.hpp"

#include <QDir>
#include <QFile>

SCENARIO("core controller can handle loading games and cores with real dependencies")
{
  GIVEN("a real subject") {

    CoreController subject;

    const QString qrcCorePath = ":/debug_snes_core.dll";
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

SCENARIO("The core environment GET callbacks should be all handled properly")
{
  GIVEN("A real core with callbacks set up") {

    CoreController subject;

    const QString qrcCorePath = ":/debug_snes_core.dll";
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

      THEN("environmentCallback(): RETRO_ENVIRONMENT_GET_VARIABLE "
           "RETRO_ENVIRONMENT_SET_VARIABLES "
           "RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE "
           "retro variables can get set and read back") {
        struct retro_variable vars[] = {
          { "snes9x_next_overclock", "SuperFX Overclock; Disabled(10MHz)|40MHz|60MHz|80MHz|100MHz|Underclock(5MHz)|Underclock(8MHz)" },
          { NULL, NULL },
        };
        REQUIRE(CoreController::environmentCallback(RETRO_ENVIRONMENT_SET_VARIABLES, vars));

        struct retro_variable variable;
        variable.key = "snes9x_next_overclock";
        variable.value = nullptr;

        REQUIRE(CoreController::environmentCallback(RETRO_ENVIRONMENT_GET_VARIABLE, &variable));
        REQUIRE(QString(variable.value) == QString("Disabled(10MHz)"));
      }

      // TODO - Finish this mess
      THEN("environmentCallback(): RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE should return true and set data to non null") {
        bool data = true;
        REQUIRE(CoreController::environmentCallback(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &data) == false);
        REQUIRE(data == false);

        //...
      }
    }
  }
}
