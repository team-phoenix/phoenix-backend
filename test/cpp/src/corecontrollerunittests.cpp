#include "catch.hpp"

#include "corecontroller.hpp"

#include <QDebug>

SCENARIO("The core controller can handle loading a game and core")
{
  GIVEN("") {
    struct MockMemory {
      void writeVideoFrame(const char*, uint, uint, uint) {}
      bool isOpened() { return true; }
      bool open(int) { return true; }
      void clear() {}
      void readKeyboardStates(qint16*, size_t) {}
    };

    static retro_game_info retroLoadGameArg = {};
    static retro_system_info systemInfoArg = {};

    static bool setAudioSampleCb = false;
    static bool setAudioBatchCb = false;
    static bool setEnvCb = false;
    static bool setInputPollCb = false;
    static bool setInputStateCb = false;
    static bool setVideoCb = false;

    struct MockCore {
      MockCore()
      {
        retro_init = []() {};
        retro_set_audio_sample = [](retro_audio_sample_t) {
          setAudioSampleCb = true;
        };
        retro_set_audio_sample_batch = [](retro_audio_sample_batch_t) {
          setAudioBatchCb = true;
        };
        retro_set_controller_port_device = [](unsigned, unsigned) {};
        retro_set_environment = [](retro_environment_t) {
          setEnvCb = true;
        };
        retro_set_input_poll = [](retro_input_poll_t) {
          setInputPollCb = true;
        };
        retro_set_input_state = [](retro_input_state_t) {
          setInputStateCb = true;
        };
        retro_set_video_refresh = [](retro_video_refresh_t) {
          setVideoCb = true;
        };
        retro_get_system_info = [](struct retro_system_info * info) {
          info->library_name = "fake library";
          info->library_version = "fake version";
          info->valid_extensions = "fake exts";
          info->need_fullpath = true;
          info->block_extract = true;
        };
        retro_load_game = [](const struct retro_game_info * info) -> bool {
          retroLoadGameArg = *info;
          return true;
        };
        retro_get_system_av_info = [](struct retro_system_av_info * avInfo) {
          avInfo->geometry.aspect_ratio = 1.0;
          avInfo->geometry.base_height = 1;
          avInfo->geometry.base_width = 2;
          avInfo->geometry.max_height = 3;
          avInfo->geometry.max_width = 4;
          avInfo->timing.fps = 60.0;
          avInfo->timing.sample_rate = 44100.0;
        };

        retro_run = []() {};
      }

      void load(QString) {}
      bool unload() { return true;}

      void (*retro_set_audio_sample)(retro_audio_sample_t);
      void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
      void (*retro_set_controller_port_device)(unsigned, unsigned);
      void (*retro_set_environment)(retro_environment_t);
      void (*retro_set_input_poll)(retro_input_poll_t);
      void (*retro_set_input_state)(retro_input_state_t);
      void (*retro_set_video_refresh)(retro_video_refresh_t);

      void (*retro_init)(void);

      void (*retro_get_system_info)(struct retro_system_info*);
      bool (*retro_load_game)(const struct retro_game_info*);
      void (*retro_get_system_av_info)(struct retro_system_av_info*);

      void (*retro_run)(void);

      void clear() {}
    };

    static const char* gameFilePath = "game/file/path.rom";
    static const char* gameBufferData = "game/buffer/data";
    static const int gameBufferSize = 256;
    static bool closeCalled = false;
    static bool copyToRamCalled = false;

    struct MockGame {
      void open(QString) {}
      void close() { closeCalled = true; }
      const char* constData() const { return gameBufferData; }
      int size() const { return gameBufferSize; }
      const char* filePath() const { return gameFilePath; }
      void copyToRam() { copyToRamCalled = true; }
      void clear() {}
    };

    struct MockInputManager {
      void poll() {}
      qint16* getKeyboardBuffer() const { return nullptr; }
      int getKeyboardBufferSize() const { return 0; }
      void updateControllerStates() {}
      qint16 getInputState(unsigned, unsigned, unsigned, unsigned) { return 0; }
    };

    using Subject = CoreController_T<MockMemory, MockCore, MockGame, MockInputManager>;
    Subject subject;
    WHEN("init() is called with a core and game path") {
      REQUIRE_NOTHROW(subject.init("/core/path", "/game/path"));

      THEN("setCallbacks() is called and fills in the callbacks")  {
        REQUIRE(setAudioSampleCb == true);
        REQUIRE(setAudioBatchCb == true);
        REQUIRE(setInputPollCb == true);
        REQUIRE(setInputStateCb == true);
        REQUIRE(setVideoCb == true);
        REQUIRE(setEnvCb == true);
      }
    }

    WHEN("openGame() is called with 'need_fullpath = true'") {
      retro_system_info systemInfo = {};
      systemInfo.need_fullpath = true;

      THEN("the game info path is filled in") {
        retro_game_info gameInfo = subject.openGame(systemInfo);
        REQUIRE(gameInfo.path == gameFilePath);
        REQUIRE(gameInfo.data == nullptr);
        REQUIRE(gameInfo.size == 0);
        REQUIRE(QString(gameInfo.meta) == "");

        REQUIRE(closeCalled == false);
        REQUIRE(copyToRamCalled == false);

        REQUIRE(retroLoadGameArg.data == gameInfo.data);
        REQUIRE(QString(retroLoadGameArg.meta) == QString(gameInfo.meta));
        REQUIRE(QString(retroLoadGameArg.path) == QString(gameInfo.path));
        REQUIRE(retroLoadGameArg.size == gameInfo.size);
      }
    }

    WHEN("openGame() is called with 'need_fullpath = false'") {
      retro_system_info systemInfo = {};
      systemInfo.need_fullpath = false;

      THEN("the game info path is filled in") {
        retro_game_info gameInfo = subject.openGame(systemInfo);
        REQUIRE(gameInfo.path == nullptr);
        REQUIRE(gameInfo.data == gameBufferData);
        REQUIRE(gameInfo.size == gameBufferSize);
        REQUIRE(QString(gameInfo.meta) == "");

        REQUIRE(closeCalled == true);
        REQUIRE(copyToRamCalled == true);

        REQUIRE(retroLoadGameArg.data == gameInfo.data);
        REQUIRE(QString(retroLoadGameArg.meta) == QString(gameInfo.meta));
        REQUIRE(QString(retroLoadGameArg.path) == QString(gameInfo.path));
        REQUIRE(retroLoadGameArg.size == gameInfo.size);
      }
    }

    WHEN("fillSystemInfo() is called") {
      Subject::SystemInfo info;
      subject.fillSystemInfo(info);

      THEN("retro_system_info is filled in") {
        REQUIRE(QString(info.systemInfo.library_name) == QString("fake library"));
        REQUIRE(QString(info.systemInfo.library_version) == QString("fake version"));
        REQUIRE(QString(info.systemInfo.valid_extensions) == QString("fake exts"));
        REQUIRE(info.systemInfo.need_fullpath == true);
        REQUIRE(info.systemInfo.block_extract == true);
      }

      THEN("retro_game_geometry is filled in") {
        REQUIRE(info.avInfo.geometry.aspect_ratio == 1.0);
        REQUIRE(info.avInfo.geometry.base_height == 1);
        REQUIRE(info.avInfo.geometry.base_width == 2);
        REQUIRE(info.avInfo.geometry.max_height == 3);
        REQUIRE(info.avInfo.geometry.max_width == 4);
      }

      THEN("retro_system_timing is filled in") {
        REQUIRE(info.avInfo.timing.fps == 60.0);
        REQUIRE(info.avInfo.timing.sample_rate == 44100.0);
      }

      THEN("isEmpty is set to false") {
        REQUIRE(info.isEmpty == false);
      }
    }

    WHEN("run() calls Core::run()") {
      subject.run();
    }

    WHEN("fini() calls Game::close()") {
      subject.fini();
    }

    WHEN("instance() is called") {
      Subject* subjectInstanceOne = &subject.instance();
      Subject* subjectInstanceTwo = &subject.instance();
      THEN("it only returns a single instance") {
        REQUIRE(subjectInstanceOne == subjectInstanceTwo);
      }
    }

  }
}
