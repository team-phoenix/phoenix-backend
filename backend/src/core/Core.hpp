#pragma once

#include "libretro.h"
#include "Logging.h"

#include <QLibrary>
#include <QFile>

// Helper for resolving libretro methods
#define RESOLVE_SYM( sym ) sym = reinterpret_cast<decltype( sym )>( library.resolve( #sym ) )

template<typename Library = QLibrary>
class Core_T
{
public:
  Core_T()
  {
    clear();
  }

  ~Core_T()
  {
    clear();
  }

  void load(const QString &filePath)
  {
    library.setFileName(filePath);

    if (library.load()) {
      resolveSymbols();
    } else {
      throw std::runtime_error("could not load " + filePath.toStdString());
    }
  }

  bool isLoaded() const
  {
    return library.isLoaded();
  }

  bool unload()
  {
    clear();
    return library.unload();
  }

  QString fileName() const
  {
    return library.fileName();
  }

  // Libretro core functions

  unsigned(*retro_api_version)(void);
  void (*retro_cheat_reset)(void);
  void (*retro_cheat_set)(unsigned, bool, const char*);
  void (*retro_deinit)(void);
  void* (*retro_get_memory_data)(unsigned);
  size_t (*retro_get_memory_size)(unsigned);
  unsigned(*retro_get_region)(void);
  void (*retro_get_system_av_info)(struct retro_system_av_info*);
  void (*retro_get_system_info)(struct retro_system_info*);
  void (*retro_init)(void);

  bool (*retro_load_game)(const struct retro_game_info*);
  bool (*retro_load_game_special)(unsigned, const struct retro_game_info*, size_t);
  void (*retro_reset)(void);
  void (*retro_run)(void);
  bool (*retro_serialize)(void*, size_t);
  size_t (*retro_serialize_size)(void);
  void (*retro_unload_game)(void);
  bool (*retro_unserialize)(const void*, size_t);

  // Frontend-defined callbacks
  void (*retro_set_audio_sample)(retro_audio_sample_t);
  void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
  void (*retro_set_controller_port_device)(unsigned, unsigned);
  void (*retro_set_environment)(retro_environment_t);
  void (*retro_set_input_poll)(retro_input_poll_t);
  void (*retro_set_input_state)(retro_input_state_t);
  void (*retro_set_video_refresh)(retro_video_refresh_t);

  // Optional core-defined callbacks
  void (*retro_audio)();
  void (*retro_audio_set_state)(bool enabled);
  void (*retro_frame_time)(retro_usec_t delta);
  void (*retro_keyboard_event)(bool down, unsigned keycode, uint32_t character,
                               uint16_t key_modifiers);
  void (*retro_hw_context_reset)(void);

  void clear()
  {
    retro_api_version = nullptr;
    retro_cheat_reset = nullptr;
    retro_cheat_set = nullptr;
    retro_deinit = nullptr;
    retro_init = nullptr;
    retro_get_memory_data = nullptr;
    retro_get_memory_size = nullptr;
    retro_get_region = nullptr;
    retro_get_system_av_info = nullptr;
    retro_get_system_info = nullptr;
    retro_load_game = nullptr;
    retro_load_game_special = nullptr;
    retro_reset = nullptr;
    retro_run = nullptr;
    retro_serialize = nullptr;
    retro_serialize_size = nullptr;
    retro_unload_game = nullptr;
    retro_unserialize = nullptr;

    retro_set_audio_sample = nullptr;
    retro_set_audio_sample_batch = nullptr;
    retro_set_controller_port_device = nullptr;
    retro_set_environment = nullptr;
    retro_set_input_poll = nullptr;
    retro_set_input_state = nullptr;
    retro_set_video_refresh = nullptr;

    retro_audio = nullptr;
    retro_audio_set_state = nullptr;
    retro_frame_time = nullptr;
    retro_keyboard_event = nullptr;
  }

  void resolveSymbols()
  {
    RESOLVE_SYM(retro_set_environment);
    RESOLVE_SYM(retro_set_video_refresh);
    RESOLVE_SYM(retro_set_audio_sample);
    RESOLVE_SYM(retro_set_audio_sample_batch);
    RESOLVE_SYM(retro_set_input_poll);
    RESOLVE_SYM(retro_set_input_state);
    RESOLVE_SYM(retro_init);
    RESOLVE_SYM(retro_deinit);
    RESOLVE_SYM(retro_api_version);
    RESOLVE_SYM(retro_get_system_info);
    RESOLVE_SYM(retro_get_system_av_info);
    RESOLVE_SYM(retro_set_controller_port_device);
    RESOLVE_SYM(retro_reset);
    RESOLVE_SYM(retro_run);
    RESOLVE_SYM(retro_serialize);
    RESOLVE_SYM(retro_serialize_size);
    RESOLVE_SYM(retro_unserialize);
    RESOLVE_SYM(retro_cheat_reset);
    RESOLVE_SYM(retro_cheat_set);
    RESOLVE_SYM(retro_load_game);
    RESOLVE_SYM(retro_load_game_special);
    RESOLVE_SYM(retro_unload_game);
    RESOLVE_SYM(retro_get_region);
    RESOLVE_SYM(retro_get_memory_data);
    RESOLVE_SYM(retro_get_memory_size);
  }

private:
  Library library;
};

using Core = Core_T<>;
