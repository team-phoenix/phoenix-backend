#include "coresymbols.h"

#include <QFile>
#include <QLibrary>

// Helper for resolving libretro methods
#define resolved_sym( fileName, sym ) sym = reinterpret_cast<decltype( sym )>( fileName.resolve( #sym ) )


CoreSymbols::~CoreSymbols() {
    clear();
}

void CoreSymbols::resolveSymbols(QLibrary &t_lib) {
    resolved_sym( t_lib, retro_set_environment );
    resolved_sym( t_lib, retro_set_video_refresh );
    resolved_sym( t_lib, retro_set_audio_sample );
    resolved_sym( t_lib, retro_set_audio_sample_batch );
    resolved_sym( t_lib, retro_set_input_poll );
    resolved_sym( t_lib, retro_set_input_state );
    resolved_sym( t_lib, retro_init );
    resolved_sym( t_lib, retro_deinit );
    resolved_sym( t_lib, retro_api_version );
    resolved_sym( t_lib, retro_get_system_info );
    resolved_sym( t_lib, retro_get_system_av_info );
    resolved_sym( t_lib, retro_set_controller_port_device );
    resolved_sym( t_lib, retro_reset );
    resolved_sym( t_lib, retro_run );
    resolved_sym( t_lib, retro_serialize );
    resolved_sym( t_lib, retro_serialize_size );
    resolved_sym( t_lib, retro_unserialize );
    resolved_sym( t_lib, retro_cheat_reset );
    resolved_sym( t_lib, retro_cheat_set );
    resolved_sym( t_lib, retro_load_game );
    resolved_sym( t_lib, retro_load_game_special );
    resolved_sym( t_lib, retro_unload_game );
    resolved_sym( t_lib, retro_get_region );
    resolved_sym( t_lib, retro_get_memory_data );
    resolved_sym( t_lib, retro_get_memory_size );

}

void CoreSymbols::clear() {
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


