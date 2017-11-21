#pragma once

#include "catch.hpp"
#include "sharedmemorybuffer.h"
#include "core.hpp"
#include "game.hpp"
#include "inputmanager.hpp"
#include "logging.h"

#include <QDebug>

template<typename Memory = SharedMemoryBuffer,
         typename DylibCore = Core,
         typename GameRom = Game,
         typename InputStateManager = InputManager>
class CoreController_T
{
public:
  CoreController_T() = default;
  ~CoreController_T() = default;

  struct SystemInfo {
    SystemInfo() = default;
    retro_system_info systemInfo{};
    retro_system_av_info avInfo{};
    bool isEmpty{ true };
  };

  template<typename AvInfo = retro_system_av_info>
  SystemInfo init(QString corePath, QString gamePath)
  {
    try {
      dylibCore.load(corePath);
      game.open(gamePath);

      setCallbacks();

      dylibCore.retro_init();

      SystemInfo systemInfo;
      fillSystemInfo(systemInfo);

      // We need to flush stderr, because some cores write to it, even though
      // the log callback exists; it's complete madness!!!.
      fflush(stderr);

      return systemInfo;

    } catch (std::runtime_error error) {
      qCDebug(phxCore) << error.what();
    }

    return SystemInfo();
  }

  void fini()
  {
    game.close();
    game.clear();
    dylibCore.unload();
    memory.clear();
  }

  void reset()
  {
    fini();
  }

  void run()
  {
    dylibCore.retro_run();
  }

  retro_game_info openGame(const retro_system_info &systemInfo)
  {
    retro_game_info gameInfo = {};

    if (systemInfo.need_fullpath) {
      gameInfo.path = game.filePath();
      gameInfo.data = nullptr;
      gameInfo.size = 0;
      gameInfo.meta = "";
    } else {
      game.copyToRam();

      gameInfo.path = nullptr;
      gameInfo.data = game.constData();
      gameInfo.size = game.size();
      gameInfo.meta = "";

      game.close();
    }

    dylibCore.retro_load_game(&gameInfo);

    return gameInfo;
  }

  void fillSystemInfo(SystemInfo &info)
  {
    dylibCore.retro_get_system_info(&info.systemInfo);

    openGame(info.systemInfo);

    dylibCore.retro_get_system_av_info(&info.avInfo);

//      emit m_audioController.audioFmtChanged(systemAvInfo.timing.fps, systemAvInfo.timing.sample_rate);

//    qCDebug(phxCore).nospace() << "coreFPS: " << info.avInfo.timing.fps;
//    qCDebug(phxCore).nospace() << "aspectRatio: " << info.avInfo.geometry.aspect_ratio;
//    qCDebug(phxCore).nospace() << "baseHeight: " << info.avInfo.geometry.base_height;
//    qCDebug(phxCore).nospace() << "baseWidth: " << info.avInfo.geometry.base_width;

//      emit initialized();

//      setEmuState(State::Initialized);
//      sendVideoInfo();
//      sendVariables();

    info.isEmpty = false;
  }

private:
  void setCallbacks()
  {
    dylibCore.retro_set_environment(CoreController_T::environmentCallback);
    dylibCore.retro_set_audio_sample(&CoreController_T::audioSampleCallback);
    dylibCore.retro_set_audio_sample_batch(&CoreController_T::audioSampleBatchCallback);
    dylibCore.retro_set_input_poll(&CoreController_T::inputPollCallback);
    dylibCore.retro_set_input_state(&CoreController_T::inputStateCallback);
    dylibCore.retro_set_video_refresh(&CoreController_T::videoRefreshCallback);
  }

  void readKeyStatesInputManager()
  {
    qint16* keyboardBuffer = inputManager.getKeyboardBuffer();
    const int keyboardBufferSize = inputManager.getKeyboardBufferSize();
    memory.readKeyboardStates(keyboardBuffer, keyboardBufferSize + sizeof(qint16));
  }

// Optional
//  static uintptr_t getFramebufferCallback(void) {

//  }

//  static retro_proc_address_t openGLProcAddressCallback(const char* sym) {

//  }

//  static bool rumbleCallback(unsigned port, enum retro_rumble_effect effect, uint16_t strength) {

//  }

private:
  Memory memory;
  DylibCore dylibCore;
  GameRom game;
  InputStateManager inputManager;

public:

  static CoreController_T &instance()
  {
    static CoreController_T controller;
    return controller;
  }

  static void audioSampleCallback(int16_t left, int16_t right)
  {
    Q_UNUSED(left);
    Q_UNUSED(right);
  }

  static size_t audioSampleBatchCallback(const int16_t* data, size_t frames)
  {
    Q_UNUSED(data);
    Q_UNUSED(frames);
    return 0;
  }

  static bool environmentCallback(unsigned cmd, void* data)
  {
    Q_UNUSED(data);

    switch (cmd) {
      default:
        break;
    }

    return false;
  }

  static void inputPollCallback(void)
  {
    instance().inputManager.poll();
    instance().readKeyStatesInputManager();
  }

//  static void logCallback(enum retro_log_level level, const char* fmt, ...) {

//  }

  static int16_t inputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id)
  {
    Q_UNUSED(port);
    Q_UNUSED(device);
    Q_UNUSED(index);
    Q_UNUSED(id);

    return 0;
  }

  static void videoRefreshCallback(const void* data, unsigned width, unsigned height, size_t pitch)
  {
    if (!instance().memory.isOpened()) {
      instance().memory.open(sizeof(bool) + (sizeof(uint) * 3) + (pitch * height * sizeof(char)));
    }

    instance().memory.writeVideoFrame(static_cast<const char*>(data), width, height, pitch);
  }

};

using CoreController = CoreController_T<>;