#include "doctest.hpp"

#include "core.hpp"

void some_func() {}

SCENARIO("A core can load a valid core path with a mocked library")
{
  GIVEN("a mocked out library") {

    WHEN("load() is called with a valid core") {
      struct MockLibrary {
        using funcPtr = void(*)();
        void setFileName(QString) {}
        bool load() { return true; }
        bool isLoaded() { return true; }
        funcPtr resolve(const char*) { return some_func; }
      };

      Core_T<MockLibrary> subject;
      REQUIRE_NOTHROW(subject.load("some/good/path.core"));

      THEN("retro_cheat_reset has been resolved, alias for all functions") {
        REQUIRE(subject.retro_cheat_reset == some_func);
      }

      THEN("clear() resets all of the symbol methods to nullptr") {
        subject.clear();

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

    WHEN("load() is called with an invalid core") {
      struct MockLibrary {
        using funcPtr = void(*)();
        void setFileName(QString) {}
        bool load() { return false; }
        bool isLoaded() { return false; }
        funcPtr resolve(const char*) { return some_func; }
      };

      Core_T<MockLibrary> subject;
      REQUIRE_THROWS_AS(subject.load("a/bad/path.core"), std::runtime_error);
    }
  }
}
