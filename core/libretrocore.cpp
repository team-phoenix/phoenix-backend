#include "libretrocore.h"

LibretroCore::LibretroCore( Core *parent ): Core( parent ) {

    // All this does is print the state change
    connect( this, &LibretroCore::stateChanged, this, &Core::setState );

    // Allocate buffer pools

    LibretroCore::state = STOPPED;
    emit stateChanged( STOPPED );
}

// Slots

void LibretroCore::setSource( QMap<QString, QString> source ) {
    coreFile.setFileName( source[ "core" ] );
    gameFile.setFileName( source[ "game" ] );
    systemPath.setPath( source[ "systemPath" ] );
    savePath.setPath( source[ "savePath" ] );
}

void LibretroCore::load() {
    state = LOADING;
    emit stateChanged( LOADING );

    // Set paths (QFileInfo gives you convenience functions, for example to extract just the directory from a file path)
    coreFileInfo.setFile( source[ "core" ] );
    gameFileInfo.setFile( source[ "game" ] );
    systemPathInfo.setFile( source[ "systemPath" ] );
    savePathInfo.setFile( source[ "savePath" ] );

    coreFile.setFileName( coreFileInfo.absoluteFilePath() );
    gameFile.setFileName( gameFileInfo.absoluteFilePath() );

    contentPath.setPath( gameFileInfo.absolutePath() );
    systemPath.setPath( systemPathInfo.absolutePath() );
    savePath.setPath( savePathInfo.absolutePath() );

    // Convert to C-style ASCII strings (needed by the API)
    corePathCString = coreFileInfo.absolutePath().toLocal8Bit().constData();
    gameFileCString = gameFileInfo.absoluteFilePath().toLocal8Bit().constData();
    gamePathCString = gameFileInfo.absolutePath().toLocal8Bit().constData();
    systemPathCString = systemPathInfo.absolutePath().toLocal8Bit().constData();
    savePathCString = savePathInfo.absolutePath().toLocal8Bit().constData();

    // Load core
    {
        qCDebug( phxCore ) << QStringLiteral( "Loading core:" ) << coreFileInfo.absoluteFilePath();

        coreFile.load();

        // Resolve symbols
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

        // Set callbacks
        symbols.retro_set_environment( environmentCallback );
        symbols.retro_set_audio_sample( audioSampleCallback );
        symbols.retro_set_audio_sample_batch( audioSampleBatchCallback );
        symbols.retro_set_input_poll( inputPollCallback );
        symbols.retro_set_input_state( inputStateCallback );
        symbols.retro_set_video_refresh( videoRefreshCallback );

        // Init the core
        symbols.retro_init();

        // Get some info about the game
        symbols.retro_get_system_info( systemInfo );
    }

    // Load game
    {
        qCDebug( phxCore ) << QStringLiteral( "Loading game:" ) << gameFileInfo.absoluteFilePath();

        // Argument struct for symbols.retro_load_game()
        retro_game_info gameInfo;

        // Full path needed, simply pass the game's file path to the core
        if( systemInfo->need_fullpath ) {
            gameInfo.path = gameFileCString;
            gameInfo.data = nullptr;
            gameInfo.size = 0;
            gameInfo.meta = "";
        }

        // Full path not needed, read the file to a buffer and pass that to the core
        else {
            gameFile.open( QIODevice::ReadOnly );

            // read into memory
            gameData = gameFile.readAll();

            gameInfo.path = nullptr;
            gameInfo.data = gameData.data();
            gameInfo.size = gameFile.size();
            gameInfo.meta = "";
        }

        symbols.retro_load_game( &gameInfo );

        // Get the AV timing/dimensions/format
        symbols.retro_get_system_av_info( avInfo );

        // Allocate buffers now that we know how large to make them
        // Assume 16-bit stereo audio, 32-bit video
        for( int i = 0; i < POOL_SIZE; i++ ) {
            // Allocate a bit extra as some cores' numbers do not add up...
            audioBufferPool[i] = ( int16_t * )calloc( 1, avInfo->timing.sample_rate * 5 );
            videoBufferPool[i] = ( uchar * )calloc( 1, avInfo->geometry.max_width * avInfo->geometry.max_height * 4 );
        }
    }

    loadSaveData();

    state = PAUSED;
    emit stateChanged( PAUSED );
}

void LibretroCore::play() {
    // Start/resume the timer

    state = PLAYING;
    emit stateChanged( PLAYING );
}

void LibretroCore::pause() {
    // Stop the timer

    state = PAUSED;
    emit stateChanged( PAUSED );
}

void LibretroCore::stop() {
    state = UNLOADING;
    emit stateChanged( UNLOADING );

    // Write SRAM, free memory

    state = STOPPED;
    emit stateChanged( STOPPED );
}

// Protected

void LibretroCore::emitAudioData( void *data, int bytes ) {
    emit audioData( data, bytes );
}

void LibretroCore::emitVideoData( void *data, int bytes ) {
    emit videoData( data, bytes );
}

void LibretroCore::LibretroCore::loadSaveData() {

    saveDataBuf = symbols.retro_get_memory_data( RETRO_MEMORY_SAVE_RAM );

    QFile file( savePathInfo.absolutePath() % QStringLiteral( "/" ) % gameFileInfo.baseName() % QStringLiteral( ".srm" ) );

    if( file.open( QIODevice::ReadOnly ) ) {
        QByteArray data = file.readAll();
        memcpy( saveDataBuf, data.data(), data.size() );

        qCDebug( phxCore ) << Q_FUNC_INFO << file.fileName() << "(true)";
        file.close();
    }

    else {
        qCDebug( phxCore ) << Q_FUNC_INFO << file.fileName() << "(false)";
    }

}

void LibretroCore::LibretroCore::storeSaveData() {

    auto localFile = savePathInfo.absolutePath() % QStringLiteral( "/" ) % gameFileInfo.baseName() % QStringLiteral( ".srm" );

    if( saveDataBuf == nullptr ) {
        qCDebug( phxCore ) << Q_FUNC_INFO << ": " << localFile << "(nullptr)";
        return;
    }

    QFile file( localFile );

    if( file.open( QIODevice::WriteOnly ) ) {
        qCDebug( phxCore ) << Q_FUNC_INFO << ": " << file.fileName();
        char *data = static_cast<char *>( saveDataBuf );
        size_t size = symbols.retro_get_memory_size( RETRO_MEMORY_SAVE_RAM );
        file.write( data, size );
        file.close();
    }

    else {
        qDebug() << Q_FUNC_INFO << ": " << file.fileName() << "(Failed)";
    }

}

// Private

void LibretroCore::audioSampleCallback( int16_t left, int16_t right ) {

    LibretroCore *core = LibretroCore::core;

    // Sanity check
    Q_ASSERT( core->audioBufferCurrentByte < core->avInfo->timing.sample_rate * 5 );

    // Stereo audio is interleaved, left then right
    core->audioBufferPool[ core->audioPoolCurrentBuffer ][ core->audioBufferCurrentByte / 2 ] = left;
    core->audioBufferPool[ core->audioPoolCurrentBuffer ][ core->audioBufferCurrentByte / 2 + 1 ] = right;

    // Each frame is 4 bytes (16-bit stereo)
    core->audioBufferCurrentByte += 4;

}

size_t LibretroCore::audioSampleBatchCallback( const int16_t *data, size_t frames ) {

    LibretroCore *core = LibretroCore::core;

    // Sanity check
    Q_ASSERT( core->audioBufferCurrentByte < core->avInfo->timing.sample_rate * 5 );

    // Need to do a bit of pointer arithmetic to get the right offset (the buffer is counted in increments of 2 bytes)
    int16_t *dst_init = core->audioBufferPool[ core->audioPoolCurrentBuffer ];
    int16_t *dst = dst_init + ( core->audioBufferCurrentByte / 2 );

    // Copy the incoming data
    memcpy( dst, data, frames * 4 );

    // Each frame is 4 bytes (16-bit stereo)
    core->audioBufferCurrentByte += frames * 4;

    return frames;

}

bool LibretroCore::environmentCallback( unsigned cmd, void *data ) {

    switch( cmd ) {
        case RETRO_ENVIRONMENT_SET_ROTATION: // 1
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_ROTATION (1)";
            break;

        case RETRO_ENVIRONMENT_GET_OVERSCAN: // 2
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_OVERSCAN (2) (handled)";
            // Crop away overscan
            return true;

        case RETRO_ENVIRONMENT_GET_CAN_DUPE: // 3
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CAN_DUPE (3) (handled)";
            *( bool * )data = true;
            return true;

        // 4 and 5 have been deprecated

        case RETRO_ENVIRONMENT_SET_MESSAGE: // 6
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_MESSAGE (6)";
            break;

        case RETRO_ENVIRONMENT_SHUTDOWN: // 7
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SHUTDOWN (7)";
            break;

        case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL: // 8
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL (8)";
            break;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: { // 9
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY (9) (handled)";
            *( const char ** )data = core->systemPathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: { // 10
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PIXEL_FORMAT (10) (handled)";

            retro_pixel_format *pixelformat = ( enum retro_pixel_format * )data;
            LibretroCore::core->pixelFormat = *pixelformat;

            switch( *pixelformat ) {
                case RETRO_PIXEL_FORMAT_0RGB1555:
                    qCDebug( phxCore ) << "\t\tPixel format: 0RGB1555\n";
                    return true;

                case RETRO_PIXEL_FORMAT_RGB565:
                    qCDebug( phxCore ) << "\t\tPixel format: RGB565\n";
                    return true;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    qCDebug( phxCore ) << "\t\tPixel format: XRGB8888\n";
                    return true;

                default:
                    qCDebug( phxCore ) << "\t\tError: Pixel format is not supported. (" << pixelformat << ")";
                    break;
            }

            return false;
        }

        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: { // 11
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS (11) (handled)";
            retro_input_descriptor descriptor = *( retro_input_descriptor * )data;
            QString key = core->inputDescriptorKey( descriptor.port, descriptor.device, descriptor.index, descriptor.id );
            core->inputDescriptors[ key ] = QString( descriptor.description );
            return true;
        }

        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: { // 12
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK (12) (handled)";
            LibretroCore::core->symbols.retro_keyboard_event = ( decltype( LibretroSymbols::retro_keyboard_event ) )data;
            break;
        }

        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: // 13
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE (13)";
            break;

        case RETRO_ENVIRONMENT_SET_HW_RENDER: // 14
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_HW_RENDER (14) (handled)";
            LibretroCore::core->openGLContext = *( retro_hw_render_callback * )data;

            switch( LibretroCore::core->openGLContext.context_type ) {
                case RETRO_HW_CONTEXT_NONE:
                    qCDebug( phxCore ) << "No hardware context was selected";
                    break;

                case RETRO_HW_CONTEXT_OPENGL:
                    qCDebug( phxCore ) << "OpenGL 2 context was selected";
                    break;

                case RETRO_HW_CONTEXT_OPENGLES2:
                    qCDebug( phxCore ) << "OpenGL ES 2 context was selected";
                    LibretroCore::core->openGLContext.context_type = RETRO_HW_CONTEXT_OPENGLES2;
                    break;

                case RETRO_HW_CONTEXT_OPENGLES3:
                    qCDebug( phxCore ) << "OpenGL 3 context was selected";
                    break;

                default:
                    qCritical() << "RETRO_HW_CONTEXT: " << LibretroCore::core->openGLContext.context_type << " was not handled";
                    break;
            }

            break;

        case RETRO_ENVIRONMENT_GET_VARIABLE: { // 15
            auto *rv = static_cast<struct retro_variable *>( data );

            if( core->variables.contains( rv->key ) ) {
                const auto &var = core->variables[rv->key];

                if( var.isValid() ) {
                    rv->value = var.value().c_str();
                }
            }

            break;
        }

        case RETRO_ENVIRONMENT_SET_VARIABLES: { // 16
            qCDebug( phxCore ) << "RETRO_ENVIRONMENT_SET_VARIABLES (16) (handled)";
            auto *rv = ( const struct retro_variable * )( data );

            for( ; rv->key != NULL; rv++ ) {
                LibretroVariable v( rv );
                core->variables.insert( v.key(), v );
                qCDebug( phxCore ) << "\t" << v;
            }

            break;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: // 17
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE_UPDATE (17)";
            break;

        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: // 18
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME (18) (handled)";

            if( !( *( const bool * )data ) ) {
                qCWarning( phxCore ) << "Core does not expect a game!";
            }

            break;

        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH: { // 19
            // This is done with the assumption that the core file path from setSource() will always be an absolute path
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LIBRETRO_PATH (19) (handled)";
            *( const char ** )data = core->corePathCString;
            break;
        }

        // 20 has been deprecated

        case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: // 21
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK (21) (handled)";
            LibretroCore::core->symbols.retro_frame_time = ( decltype( LibretroSymbols::retro_frame_time ) )data;
            break;

        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: // 22
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_AUDIO_CALLBACK (22)";
            break;

        case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: // 23
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE (23)";
            break;

        case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES: // 24
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES (24)";
            break;

        case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE: // 25
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SENSOR_INTERFACE (25)";
            break;

        case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: // 26
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CAMERA_INTERFACE (26)";
            break;

        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: { // 27
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LOG_INTERFACE (27) (handled)";
            struct retro_log_callback *logcb = ( struct retro_log_callback * )data;
            logcb->log = logCallback;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: // 28
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_PERF_INTERFACE (28)";
            break;

        case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE: // 29
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LOCATION_INTERFACE (29)";
            break;

        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY: // 30
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY (30)";
            break;

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: { // 31
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_saveDirectory (31) (handled)";
            *( const char ** )data = core->savePathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: // 32
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_systemAVInfo (32)";
            break;

        case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK: // 33
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK (33)";
            break;

        case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: // 34
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO (34)";
            break;

        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: // 35
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_CONTROLLER_INFO (35)";
            break;

        default:
            qCDebug( phxCore ) << "Error: Environment command " << cmd << " is not defined in the frontend's libretro.h!.";
            return false;
    }

    // Command was not handled
    return false;

}

void LibretroCore::inputPollCallback( void ) {

    // if( core->inputManager ) {
    //     core->inputManager->pollStates();
    // }

    // qCDebug( phxCore ) << "Core::inputPollCallback";
    return;

}

void LibretroCore::logCallback( enum retro_log_level level, const char *fmt, ... ) {

    QVarLengthArray<char, 1024> outbuf( 1024 );
    va_list args;
    va_start( args, fmt );
    int ret = vsnprintf( outbuf.data(), outbuf.size(), fmt, args );

    if( ret < 0 ) {
        qCDebug( phxCore ) << "logCallback: could not format string";
        return;
    } else if( ( ret + 1 ) > outbuf.size() ) {
        outbuf.resize( ret + 1 );
        ret = vsnprintf( outbuf.data(), outbuf.size(), fmt, args );

        if( ret < 0 ) {
            qCDebug( phxCore ) << "logCallback: could not format string";
            return;
        }
    }

    va_end( args );

    // remove trailing newline, which are already added by qCDebug
    if( outbuf.value( ret - 1 ) == '\n' ) {
        outbuf[ret - 1] = '\0';

        if( outbuf.value( ret - 2 ) == '\r' ) {
            outbuf[ret - 2] = '\0';
        }
    }

    switch( level ) {
        case RETRO_LOG_DEBUG:
            qCDebug( phxCore ) << "RETRO_LOG_DEBUG:" << outbuf.data();
            break;

        case RETRO_LOG_INFO:
            qCDebug( phxCore ) << "RETRO_LOG_INFO:" << outbuf.data();
            break;

        case RETRO_LOG_WARN:
            qCWarning( phxCore ) << "RETRO_LOG_WARN:" << outbuf.data();
            break;

        case RETRO_LOG_ERROR:
            qCCritical( phxCore ) << "RETRO_LOG_ERROR:" << outbuf.data();
            break;

        default:
            qCWarning( phxCore ) << "RETRO_LOG (unknown category!?):" << outbuf.data();
            break;
    }

}

int16_t LibretroCore::inputStateCallback( unsigned port, unsigned device, unsigned index, unsigned id ) {
    Q_UNUSED( port )
    Q_UNUSED( device )
    Q_UNUSED( index )
    Q_UNUSED( id )

    //    // we don't handle index for now...


    //    if( !core->inputManager || static_cast<int>( port ) >= core->inputManager->size() ) {
    //        return 0;
    //    }


    //    auto *inputDevice = core->inputManager->at( port );

    //    auto event = static_cast<InputDeviceEvent::Event>( id );

    //    if( port == 0 ) {
    //        auto keyState = core->inputManager->keyboard->value( event, 0 );

    //        if( !inputDevice ) {
    //            return keyState;
    //        }

    //        auto deviceState = inputDevice->value( event, 0 );
    //        return deviceState | keyState;
    //    }

    //    // make sure the InputDevice was configured
    //    // to map to the requested RETRO_DEVICE.

    //    if( !inputDevice || inputDevice->type() != static_cast<InputDevice::LibretroType>( device ) ) {
    //        return 0;
    //    }

    //    return inputDevice->value( event, 0 );
    return 0;
}

void LibretroCore::videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch ) {

    Q_UNUSED( width );

    // Current frame exists, send it on its way
    if( data ) {

        memcpy( core->videoBufferPool[core->videoPoolCurrentBuffer], data, height * pitch );
        core->emitVideoData( core->videoBufferPool[core->videoPoolCurrentBuffer], height * pitch );
        core->videoPoolCurrentBuffer = ( core->videoPoolCurrentBuffer + 1 ) % POOL_SIZE;

    }

    // Current frame is a dupe, send the last actual frame again
    else {

        core->emitVideoData( core->videoBufferPool[core->videoPoolCurrentBuffer], height * pitch );

    }

    // Flush the audio used so far
    core->emitAudioData( core->audioBufferPool[core->audioPoolCurrentBuffer], core->audioBufferCurrentByte );
    core->audioBufferCurrentByte = 0;
    core->audioPoolCurrentBuffer = ( core->audioPoolCurrentBuffer + 1 ) % POOL_SIZE;

    return;

}

QString LibretroCore::inputDescriptorKey( unsigned port, unsigned device, unsigned index, unsigned id ) {
    return QString( port ) % ',' % QString( device ) % ',' % QString( index ) % ',' % QString( id );
}
