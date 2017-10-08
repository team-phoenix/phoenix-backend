#include "libretrolibrary.h"
#include "logging.h"

#include <QFile>
#include <QLibrary>


// Helper for resolving libretro methods
#define resolved_sym( sym ) sym = reinterpret_cast<decltype( sym )>( m_library.resolve( #sym ) )


LibretroLibrary::LibretroLibrary() {
    clear();
}

LibretroLibrary::~LibretroLibrary() {
    clear();
}

bool LibretroLibrary::isLoaded() const {
    return m_library.isLoaded();
}

bool LibretroLibrary::unload() {
    clear();
    return m_library.unload();
}

QString LibretroLibrary::fileName() const {
    return m_library.fileName();
}

bool LibretroLibrary::load(const QString &t_filePath) {
    if ( !QFile::exists( t_filePath ) ) {
        qCDebug( phxSharedLibrary ) << t_filePath << "does not exist.";
        return false;
    }

    m_library.setFileName( t_filePath );

    if ( m_library.load() ) {
        resolveSymbols();
    }

    return m_library.isLoaded();
}

void LibretroLibrary::resolveSymbols() {

    qCDebug( phxSharedLibrary, "resolving symbols." );

    resolved_sym( retro_set_environment );
    resolved_sym( retro_set_video_refresh );
    resolved_sym( retro_set_audio_sample );
    resolved_sym( retro_set_audio_sample_batch );
    resolved_sym( retro_set_input_poll );
    resolved_sym( retro_set_input_state );
    resolved_sym( retro_init );
    resolved_sym( retro_deinit );
    resolved_sym( retro_api_version );
    resolved_sym( retro_get_system_info );
    resolved_sym( retro_get_system_av_info );
    resolved_sym( retro_set_controller_port_device );
    resolved_sym( retro_reset );
    resolved_sym( retro_run );
    resolved_sym( retro_serialize );
    resolved_sym( retro_serialize_size );
    resolved_sym( retro_unserialize );
    resolved_sym( retro_cheat_reset );
    resolved_sym( retro_cheat_set );
    resolved_sym( retro_load_game );
    resolved_sym( retro_load_game_special );
    resolved_sym( retro_unload_game );
    resolved_sym( retro_get_region );
    resolved_sym( retro_get_memory_data );
    resolved_sym( retro_get_memory_size );

    qCDebug( phxSharedLibrary, "symbols resolved." );

}

void LibretroLibrary::clear() {
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


