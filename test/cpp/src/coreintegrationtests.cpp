#include "catch.hpp"

#include "core.hpp"

#include <QFile>
#include <QDir>

SCENARIO("A core can load a valid core path with a real library")
{
  GIVEN("a real core") {

    const QString qrcCorePath = ":/snes9x_libretro.dll";
    const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

    const QString workingCorePath = QDir::temp().filePath("tempCore.sfc");
    const QString workingGamePath = QDir::temp().filePath("tempGame");

    QFile::copy(qrcCorePath, workingCorePath);
    QFile::copy(qrcGamePath, workingGamePath);

    REQUIRE(QFile::exists(workingCorePath) == true);
    REQUIRE(QFile::exists(workingGamePath) == true);

    Core subject;
    WHEN("load() is called with a real core") {
      REQUIRE_NOTHROW(subject.load(workingCorePath));

      THEN("all required methods have been resolved") {
        REQUIRE(subject.retro_api_version != nullptr);;
        REQUIRE(subject.retro_cheat_reset != nullptr);
        REQUIRE(subject.retro_cheat_set != nullptr);
        REQUIRE(subject.retro_deinit != nullptr);
        REQUIRE(subject.retro_get_memory_data != nullptr);
        REQUIRE(subject.retro_get_memory_size != nullptr);
        REQUIRE(subject.retro_get_region != nullptr);
        REQUIRE(subject.retro_get_system_av_info != nullptr);
        REQUIRE(subject.retro_get_system_info != nullptr);
        REQUIRE(subject.retro_init != nullptr);

        REQUIRE(subject.retro_load_game != nullptr);
        REQUIRE(subject.retro_load_game_special != nullptr);
        REQUIRE(subject.retro_reset != nullptr);
        REQUIRE(subject.retro_run != nullptr);
        REQUIRE(subject.retro_serialize != nullptr);
        REQUIRE(subject.retro_serialize_size != nullptr);
        REQUIRE(subject.retro_unload_game != nullptr);
        REQUIRE(subject.retro_unserialize != nullptr);

        // Frontend-defined callbacks
        REQUIRE(subject.retro_set_audio_sample != nullptr);
        REQUIRE(subject.retro_set_audio_sample_batch != nullptr);
        REQUIRE(subject.retro_set_controller_port_device != nullptr);;
        REQUIRE(subject.retro_set_environment != nullptr);
        REQUIRE(subject.retro_set_input_poll != nullptr);
        REQUIRE(subject.retro_set_input_state != nullptr);
        REQUIRE(subject.retro_set_video_refresh != nullptr);
      }
    }

    WHEN("unload() is called after a valid core has been loaded") {
      REQUIRE_NOTHROW(subject.load(workingCorePath));
      // TODO - Some other subject is using the core library,
      // so this will always fail to unload. Need to investigate!
      subject.unload();
      THEN("all required methods have been nulled out") {
        REQUIRE(subject.retro_api_version == nullptr);;
        REQUIRE(subject.retro_cheat_reset == nullptr);
        REQUIRE(subject.retro_cheat_set == nullptr);
        REQUIRE(subject.retro_deinit == nullptr);
        REQUIRE(subject.retro_get_memory_data == nullptr);
        REQUIRE(subject.retro_get_memory_size == nullptr);
        REQUIRE(subject.retro_get_region == nullptr);
        REQUIRE(subject.retro_get_system_av_info == nullptr);
        REQUIRE(subject.retro_get_system_info == nullptr);
        REQUIRE(subject.retro_init == nullptr);

        REQUIRE(subject.retro_load_game == nullptr);
        REQUIRE(subject.retro_load_game_special == nullptr);
        REQUIRE(subject.retro_reset == nullptr);
        REQUIRE(subject.retro_run == nullptr);
        REQUIRE(subject.retro_serialize == nullptr);
        REQUIRE(subject.retro_serialize_size == nullptr);
        REQUIRE(subject.retro_unload_game == nullptr);
        REQUIRE(subject.retro_unserialize == nullptr);

        // Frontend-defined callbacks
        REQUIRE(subject.retro_set_audio_sample == nullptr);
        REQUIRE(subject.retro_set_audio_sample_batch == nullptr);
        REQUIRE(subject.retro_set_controller_port_device == nullptr);;
        REQUIRE(subject.retro_set_environment == nullptr);
        REQUIRE(subject.retro_set_input_poll == nullptr);
        REQUIRE(subject.retro_set_input_state == nullptr);
        REQUIRE(subject.retro_set_video_refresh == nullptr);
      }
    }

    WHEN("clear() is called") {
      REQUIRE_NOTHROW(subject.load(workingCorePath));
      subject.clear();
      THEN("all required methods have been nulled out") {
        REQUIRE(subject.retro_api_version == nullptr);;
        REQUIRE(subject.retro_cheat_reset == nullptr);
        REQUIRE(subject.retro_cheat_set == nullptr);
        REQUIRE(subject.retro_deinit == nullptr);
        REQUIRE(subject.retro_get_memory_data == nullptr);
        REQUIRE(subject.retro_get_memory_size == nullptr);
        REQUIRE(subject.retro_get_region == nullptr);
        REQUIRE(subject.retro_get_system_av_info == nullptr);
        REQUIRE(subject.retro_get_system_info == nullptr);
        REQUIRE(subject.retro_init == nullptr);

        REQUIRE(subject.retro_load_game == nullptr);
        REQUIRE(subject.retro_load_game_special == nullptr);
        REQUIRE(subject.retro_reset == nullptr);
        REQUIRE(subject.retro_run == nullptr);
        REQUIRE(subject.retro_serialize == nullptr);
        REQUIRE(subject.retro_serialize_size == nullptr);
        REQUIRE(subject.retro_unload_game == nullptr);
        REQUIRE(subject.retro_unserialize == nullptr);

        // Frontend-defined callbacks
        REQUIRE(subject.retro_set_audio_sample == nullptr);
        REQUIRE(subject.retro_set_audio_sample_batch == nullptr);
        REQUIRE(subject.retro_set_controller_port_device == nullptr);;
        REQUIRE(subject.retro_set_environment == nullptr);
        REQUIRE(subject.retro_set_input_poll == nullptr);
        REQUIRE(subject.retro_set_input_state == nullptr);
        REQUIRE(subject.retro_set_video_refresh == nullptr);
      }
    }

    WHEN("load() is called with a non-existant core") {
      REQUIRE_THROWS_AS(subject.load("fakecore.core"), std::runtime_error);
    }

    QFile::remove(workingCorePath);
  }
}

