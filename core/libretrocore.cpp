#include "libretrocore.h"

LibretroCore::LibretroCore( Core *parent ): Core( parent ),
    // Protected
    symbols(),
    openGLContext(),

    // Private
    coreFile(), gameFile(),
    contentPath(), systemPath(), savePath(),
    coreFileInfo(), gameFileInfo(), systemPathInfo(), savePathInfo(),
    corePathByteArray(), gameFileByteArray(), gamePathByteArray(), systemPathByteArray(), savePathByteArray(),
    corePathCString( nullptr ), gameFileCString( nullptr ), gamePathCString( nullptr ), systemPathCString( nullptr ), savePathCString( nullptr ),
    gameData(),
    saveDataBuf( nullptr ),
    systemInfo( new retro_system_info() ),
    inputDescriptors(),
    audioBufferPool{ nullptr }, audioPoolCurrentBuffer( 0 ), audioBufferCurrentByte( 0 ),
    videoBufferPool{ nullptr }, videoPoolCurrentBuffer( 0 ),
    consumerFmt(), inputStates{ 0 },
    variables() {
    core = this;

    // All Libretro cores are pausable, just stop calling retro_run()
    pausable = true;

    currentState = Control::STOPPED;
    allPropertiesChanged();
}

LibretroCore::~LibretroCore() {
    for( int i = 0; i < POOL_SIZE; i++ ) {
        free( audioBufferPool[i] );
        free( videoBufferPool[i] );
    }
}

// Slots

void LibretroCore::load() {
    Core::setState( Control::LOADING );

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
    corePathByteArray = coreFileInfo.absolutePath().toLocal8Bit();
    gameFileByteArray = gameFileInfo.absoluteFilePath().toLocal8Bit();
    gamePathByteArray = gameFileInfo.absolutePath().toLocal8Bit();
    systemPathByteArray = systemPathInfo.absolutePath().toLocal8Bit();
    savePathByteArray = savePathInfo.absolutePath().toLocal8Bit();
    corePathCString = corePathByteArray.constData();
    gameFileCString = gameFileByteArray.constData();
    gamePathCString = gamePathByteArray.constData();
    systemPathCString = systemPathByteArray.constData();
    savePathCString = savePathByteArray.constData();

    qDebug() << "";
    qCDebug( phxCore ) << "Now loading:";
    qCDebug( phxCore ) << "Core        :" << source[ "core" ];
    qCDebug( phxCore ) << "Game        :" << source[ "game" ];
    qCDebug( phxCore ) << "System path :" << source["systemPath"];
    qCDebug( phxCore ) << "Save path   :" << source["savePath"];
    qDebug() << "";

    // Set defaults that'll get overwritten as the core loads if necessary
    {
        // Pixel format is set to QImage::Format_RGB16 by default by the struct ProducerFormat constructor
        // However, for Libretro the default is RGB1555 aka QImage::Format_RGB555
        producerFmt.videoPixelFormat = QImage::Format_RGB555;
    }

    // Load core
    {
        qCDebug( phxCore ) << "Loading core:" << coreFileInfo.absoluteFilePath();

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
        qCDebug( phxCore ) << "Loading game:" << gameFileInfo.absoluteFilePath();

        // Argument struct for symbols.retro_load_game()
        retro_game_info gameInfo;

        // Full path needed, simply pass the game's file path to the core
        if( systemInfo->need_fullpath ) {
            qCDebug( phxCore ) << "Passing file path to core...";
            gameInfo.path = gameFileCString;
            gameInfo.data = nullptr;
            gameInfo.size = 0;
            gameInfo.meta = "";
        }

        // Full path not needed, read the file to memory and pass that to the core
        else {
            qCDebug( phxCore ) << "Copying game contents to memory...";
            gameFile.open( QIODevice::ReadOnly );

            // read into memory
            gameData = gameFile.readAll();

            gameInfo.path = nullptr;
            gameInfo.data = gameData.constData();
            gameInfo.size = gameFile.size();
            gameInfo.meta = "";
        }

        symbols.retro_load_game( &gameInfo );
    }

    // Load save data
    loadSaveData();

    // Get audio/video timing and send to consumers, allocate buffer pool
    {

        // Get info from the core
        retro_system_av_info *avInfo = new retro_system_av_info();
        symbols.retro_get_system_av_info( avInfo );

        // Audio

        // The Libretro API only support 16-bit stereo PCM
        // Not sure, but I think that Libretro uses the host's native byte order for audio,
        // which for x86_64 is little endian
        producerFmt.audioFormat.setSampleSize( 16 );
        producerFmt.audioFormat.setSampleRate( avInfo->timing.sample_rate );
        producerFmt.audioFormat.setChannelCount( 2 );
        producerFmt.audioFormat.setSampleType( QAudioFormat::SignedInt );
        producerFmt.audioFormat.setByteOrder( QAudioFormat::LittleEndian );
        producerFmt.audioFormat.setCodec( "audio/pcm" );

        // inputFormat may or may not be set at this point, either way is fine
        // By default retro_run() is expected to be called at 60Hz
        producerFmt.audioRatio = consumerFmt.videoFramerate / avInfo->timing.fps;

        // Video

        producerFmt.videoBytesPerLine = avInfo->geometry.base_width *
                                        QImage().toPixelFormat( producerFmt.videoPixelFormat ).bitsPerPixel() / 8;
        producerFmt.videoFramerate = avInfo->timing.fps;

        producerFmt.videoSize.setWidth( avInfo->geometry.base_width );
        producerFmt.videoSize.setHeight( avInfo->geometry.base_height );

        qCDebug( phxCore ) << "Base video size:" << QSize( avInfo->geometry.base_width, avInfo->geometry.base_height );
        qCDebug( phxCore ) << "Maximum video size:" << QSize( avInfo->geometry.max_width, avInfo->geometry.max_height );

        emit producerFormat( producerFmt );
        emit libretroCoreNativeFramerate( avInfo->timing.fps );

        // Allocate buffers to fit this max size
        // Assume 16-bit stereo audio, 32-bit video
        for( int i = 0; i < POOL_SIZE; i++ ) {
            // Allocate a bit extra as some cores' numbers do not add up...
            audioBufferPool[ i ] = ( int16_t * )calloc( 1, avInfo->timing.sample_rate * 5 );
            videoBufferPool[ i ] = ( quint8 * )calloc( 1, avInfo->geometry.max_width * avInfo->geometry.max_height * 4 );
        }

        delete avInfo;

    }

    Core::setState( Control::PAUSED );
}

void LibretroCore::stop() {
    Core::setState( Control::UNLOADING );

    // Write SRAM

    qCInfo( phxCore ) << "=======Saving game...=======";
    storeSaveData();
    qCInfo( phxCore ) << "============================";

    // Unload core

    // symbols.retro_audio is the first symbol set to null in the constructor, so check that one
    if( symbols.retro_audio ) {
        symbols.retro_unload_game();
        symbols.retro_deinit();
        symbols.clear();
    }

    Core::setState( Control::STOPPED );
}

void LibretroCore::consumerFormat( ProducerFormat format ) {
    consumerFmt = format;

    // Update all consumers if the audio ratio changes
    if( producerFmt.audioRatio != consumerFmt.videoFramerate / producerFmt.videoFramerate ) {
        // hostFPS / coreFPS
        producerFmt.audioRatio = consumerFmt.videoFramerate / producerFmt.videoFramerate;
        // qCDebug( phxCore ).nospace() << "Updating consumers with new coreFPS... (firstFrame = " << producerFmt.firstFrame << ")";
        emit producerFormat( producerFmt );
    }
}

void LibretroCore::consumerData( QString type, QMutex *mutex, void *data, size_t bytes , qint64 timestamp ) {
    Q_UNUSED( data )
    Q_UNUSED( bytes )

    if( type == QStringLiteral( "input" ) ) {

        // Discard data that's too far from the past to matter anymore
        if( QDateTime::currentMSecsSinceEpoch() - timestamp > 100 ) {
            // qCWarning( phxCore ) << "Core is running too slow, discarding signal (printing this probably isn't helping...)";
            return;
        }

        QMutexLocker locker( mutex );

        // Copy incoming input data
        int16_t *newInputStates = ( int16_t * )data;

        for( int i = 0; i < 16; i++ ) {
            inputStates[ i ] = newInputStates[ i ];
        }

        // Run the emulator for a frame if we're supposed to
        if( currentState == Control::PLAYING ) {
            // printFPSStatistics();
            symbols.retro_run();
        }

    }
}

void LibretroCore::testDoFrame() {
    // Run the emulator for a frame if we're supposed to
    if( currentState == Control::PLAYING ) {
        // printFPSStatistics();
        symbols.retro_run();
    }
}

// Protected

LibretroCore *LibretroCore::core = nullptr;

void LibretroCore::emitAudioData( void *data, size_t bytes ) {
    emit producerData( QStringLiteral( "audio" ), &producerMutex, data, bytes, QDateTime::currentMSecsSinceEpoch() );
}

void LibretroCore::emitVideoData( void *data, unsigned width, unsigned height, size_t pitch, size_t bytes ) {

    // Cores can change the size of the video they output (within the bounds they set on load) at any time
    if( producerFmt.videoSize != QSize( width, height ) || producerFmt.videoBytesPerLine != pitch ) {

        qCDebug( phxCore ) << "Video resized!";
        qCDebug( phxCore ) << "Old video size:" << producerFmt.videoSize;
        qCDebug( phxCore ) << "New video size:" << QSize( width, height );

        producerFmt.videoBytesPerLine = pitch;
        producerFmt.videoSize.setWidth( width );
        producerFmt.videoSize.setHeight( height );

        emit producerFormat( producerFmt );

    }

    emit producerData( QStringLiteral( "video" ), &producerMutex, data, bytes, QDateTime::currentMSecsSinceEpoch() );

}

// Private

void LibretroCore::LibretroCore::loadSaveData() {
    saveDataBuf = symbols.retro_get_memory_data( RETRO_MEMORY_SAVE_RAM );

    if( !saveDataBuf ) {
        qCInfo( phxCore ) << "Unable to get save data, the core may be handling saving internally or an error happened";
    }

    QFile file( savePathInfo.absolutePath() % QStringLiteral( "/" ) % gameFileInfo.baseName() % QStringLiteral( ".sav" ) );

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
    QString localFile = savePathInfo.absolutePath() % QStringLiteral( "/" ) % gameFileInfo.baseName() % QStringLiteral( ".sav" );

    qCDebug( phxCore ).nospace() << "Saving: " << localFile;

    if( saveDataBuf == nullptr ) {
        qCDebug( phxCore ).nospace() << "Skipping, the core has done the save by itself";
        return;
    }

    QFile file( localFile );

    if( file.open( QIODevice::WriteOnly ) ) {
        char *data = static_cast<char *>( saveDataBuf );
        size_t size = symbols.retro_get_memory_size( RETRO_MEMORY_SAVE_RAM );
        file.write( data, size );
        file.close();
        qCDebug( phxCore ) << "Save successful";
    }

    else {
        qCDebug( phxCore ).nospace() << "Failed: " << file.errorString();
    }
}

void LibretroCore::audioSampleCallback( int16_t left, int16_t right ) {
    LibretroCore *core = LibretroCore::core;

    QMutexLocker locker( &core->producerMutex );

    // Sanity check
    Q_ASSERT_X( core->audioBufferCurrentByte < core->producerFmt.audioFormat.sampleRate() * 5,
                "audio batch callback", QString( "Buffer pool overflow (%1)" ).arg( core->audioBufferCurrentByte ).toLocal8Bit() );

    // Stereo audio is interleaved, left then right
    core->audioBufferPool[ core->audioPoolCurrentBuffer ][ core->audioBufferCurrentByte / 2 ] = left;
    core->audioBufferPool[ core->audioPoolCurrentBuffer ][ core->audioBufferCurrentByte / 2 + 1 ] = right;

    // Each frame is 4 bytes (16-bit stereo)
    core->audioBufferCurrentByte += 4;
}

size_t LibretroCore::audioSampleBatchCallback( const int16_t *data, size_t frames ) {
    LibretroCore *core = LibretroCore::core;

    QMutexLocker locker( &core->producerMutex );

    // Sanity check
    Q_ASSERT_X( core->audioBufferCurrentByte < core->producerFmt.audioFormat.sampleRate() * 5,
                "audio batch callback", QString( "Buffer pool overflow (%1)" ).arg( core->audioBufferCurrentByte ).toLocal8Bit() );

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

            switch( *pixelformat ) {
                case RETRO_PIXEL_FORMAT_0RGB1555:
                    core->producerFmt.videoPixelFormat = QImage::Format_RGB555;
                    qCDebug( phxCore ) << "\t\tPixel format: 0RGB1555 aka QImage::Format_RGB555";
                    return true;

                case RETRO_PIXEL_FORMAT_RGB565:
                    core->producerFmt.videoPixelFormat = QImage::Format_RGB16;
                    qCDebug( phxCore ) << "\t\tPixel format: RGB565 aka QImage::Format_RGB16";
                    return true;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    core->producerFmt.videoPixelFormat = QImage::Format_RGB32;
                    qCDebug( phxCore ) << "\t\tPixel format: XRGB8888 aka QImage::Format_RGB32";
                    return true;

                default:
                    qCCritical( phxCore ) << "\t\tError: Pixel format is not supported. ("
                                          << pixelformat << ")";
                    break;
            }

            return false;
        }

        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: { // 11
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS (11) (handled)";

            for( retro_input_descriptor *descriptor = ( retro_input_descriptor * )data; descriptor->description; descriptor++ ) {
                QString key = core->inputTupleToString( descriptor->port, descriptor->device, descriptor->index, descriptor->id );
                core->inputDescriptors[ key ] = QString( descriptor->description );
                qCDebug( phxCore ) << "\t\t" << key << descriptor->description;
            }

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

            core->producerFmt.videoMode = HARDWARERENDER;

            core->openGLContext = *( retro_hw_render_callback * )data;

            switch( core->openGLContext.context_type ) {
                case RETRO_HW_CONTEXT_NONE:
                    qCDebug( phxCore ) << "\t\tNo hardware context was selected";
                    break;

                case RETRO_HW_CONTEXT_OPENGL:
                    qCDebug( phxCore ) << "\t\tOpenGL 2 context was selected";
                    break;

                case RETRO_HW_CONTEXT_OPENGLES2:
                    qCDebug( phxCore ) << "\t\tOpenGL ES 2 context was selected";
                    break;

                case RETRO_HW_CONTEXT_OPENGLES3:
                    qCDebug( phxCore ) << "\t\tOpenGL 3 context was selected";
                    break;

                default:
                    qCritical() << "\t\tRETRO_HW_CONTEXT: " << core->openGLContext.context_type << " was not handled!";
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
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SENSOR_INTERFACE (RETRO_ENVIRONMENT_EXPERIMENTAL)(25)";
            break;

        case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: // 26
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CAMERA_INTERFACE (RETRO_ENVIRONMENT_EXPERIMENTAL)(26)";
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

        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY: { // 30
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY (30)";
            *( const char ** )data = core->systemPathCString;
            return true;
        }
        break;

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: { // 31
            qCDebug( phxCore ) << QStringLiteral( "\tRETRO_ENVIRONMENT_GET_saveDirectory (31) (handled)" );
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

        case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: //36
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_MEMORY_MAPS (RETRO_ENVIRONMENT_EXPERIMENTAL)(36)";
            break;

        default:
            qCDebug( phxCore ) << "Error: Environment command " << cmd << " is not defined in the frontend's libretro.h!.";
            return false;

    }

    // Command was not handled
    return false;

}

void LibretroCore::inputPollCallback( void ) {
    // Do nothing, this is handled before retro_run() is called
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
    Q_UNUSED( index )

    // TODO: Support more than the default joypad
    if( device != RETRO_DEVICE_JOYPAD ) {
        return 0;
    }

    // TODO: ...and their buttons
    if( id > 15 ) {
        return 0;
    }

    int16_t value = ( ( core->inputStates[ port ] >> id ) & 0x01 );

    if( port == 0 ) {
        //qCDebug( phxCore ) << "Valid input request" << port << device << index << id << "Value:" << value << "Raw:" << core->inputStates[ port ];
    }

    return value;
}

void LibretroCore::videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch ) {
    Q_UNUSED( width );

    // Current frame exists, send it on its way
    if( data ) {

        core->producerMutex.lock();
        memcpy( core->videoBufferPool[ core->videoPoolCurrentBuffer ], data, height * pitch );
        core->producerMutex.unlock();

        core->emitVideoData( core->videoBufferPool[ core->videoPoolCurrentBuffer ], width, height, pitch, pitch * height );
        core->videoPoolCurrentBuffer = ( core->videoPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }

    // Current frame is a dupe, send the last actual frame again
    else {
        core->emitVideoData( core->videoBufferPool[ core->videoPoolCurrentBuffer ], width, height, pitch, pitch * height );
    }

    // Flush the audio used so far
    core->emitAudioData( core->audioBufferPool[ core->audioPoolCurrentBuffer ], core->audioBufferCurrentByte );
    core->audioBufferCurrentByte = 0;
    core->audioPoolCurrentBuffer = ( core->audioPoolCurrentBuffer + 1 ) % POOL_SIZE;

    return;
}

QString LibretroCore::inputTupleToString( unsigned port, unsigned device, unsigned index, unsigned id ) {
    return QString::number( port, 16 ) % ',' % QString::number( device, 16 ) % ','
           % QString::number( index, 16 ) % ',' % QString::number( id, 16 );
}
