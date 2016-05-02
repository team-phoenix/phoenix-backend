#include "libretrocore.h"

LibretroCore::LibretroCore( Core *parent ): Core( parent ),
    // Private
    systemInfo( new retro_system_info() ) {
    core = this;

    // All Libretro cores are pausable, just stop calling retro_run()
    pausable = true;
}

LibretroCore::~LibretroCore() {
    for( int i = 0; i < POOL_SIZE; i++ ) {
        delete audioBufferPool[i];
        delete videoBufferPool[i];
    }
}

// Slots

void LibretroCore::commandIn( Command command, QVariant data, qint64 timeStamp ) {
    // Command is not relayed to children automatically

    switch( command ) {
        case Command::Load: {
            qCDebug( phxCore ) << command;
            state = State::Loading;
            emit commandOut( Command::Load, QVariant(), QDateTime::currentMSecsSinceEpoch() );

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
            qCDebug( phxCore ) << "System path :" << source[ "systemPath" ];
            qCDebug( phxCore ) << "Save path   :" << source[ "savePath" ];
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

                qDebug() << "";
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

                qDebug() << "";
            }

            // Load save data
            loadSaveData();

            // Get audio/video timing and send to consumers, allocate buffer pool
            {
                // Get info from the core
                retro_system_av_info *avInfo = new retro_system_av_info();
                symbols.retro_get_system_av_info( avInfo );
                getAVInfo( avInfo );
                allocateBufferPool( avInfo );
                emit commandOut( Command::CoreFPS, ( qreal )( avInfo->timing.fps ), QDateTime::currentMSecsSinceEpoch() );
                delete avInfo;
            }

            // Set all variables to their defaults, mark all variables as dirty
            {
                for( auto key : variables.keys() ) {
                    LibretroVariable &variable = variables[ key ];

                    if( !variable.choices().size() ) {
                        continue;
                    }

                    // Assume the defualt choice to be the first option offered
                    std::string defaultChoice = variable.choices().at( 0 );

                    if( !strlen( defaultChoice.c_str() ) ) {
                        continue;
                    }

                    // Assign
                    variable.setValue( defaultChoice );

                }

                variablesHaveChanged = true;
            }

            pausable = true;
            emit commandOut( Command::SetPausable, true, QDateTime::currentMSecsSinceEpoch() );

            state = State::Paused;
            emit commandOut( Command::Pause, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            break;
        }

        case Command::Play: {
            qCDebug( phxCore ) << command;
            state = State::Playing;
            emit commandOut( Command::Play, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            break;
        }

        case Command::Pause: {
            qCDebug( phxCore ) << command;
            state = State::Paused;
            emit commandOut( Command::Pause, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            break;
        }

        case Command::Stop: {
            qCDebug( phxCore ) << command;
            state = State::Unloading;
            emit commandOut( Command::Unload, QVariant(), QDateTime::currentMSecsSinceEpoch() );

            // Write SRAM

            qCInfo( phxCore ) << "=======Saving game...=======";
            storeSaveData();
            qCInfo( phxCore ) << "============================";

            // Unload core

            // symbols.retro_api_version is reasonably expected to be defined if the core is loaded
            if( symbols.retro_api_version ) {
                symbols.retro_unload_game();
                symbols.retro_deinit();
                symbols.clear();
                coreFile.unload();
                qCDebug( phxCore ) << "Unloaded core successfully";
            } else {
                qCCritical( phxCore ) << "stop() called on an unloaded core!";
            }

            state = State::Stopped;
            emit commandOut( Command::Stop, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            break;
        }

        // Run the emulator for a frame if we're supposed to
        case Command::Heartbeat: {
            // Drop any heartbeats from too far in the past
            if( QDateTime::currentMSecsSinceEpoch() - timeStamp > 50 ) {
                return;
            }

            emit commandOut( command, data, timeStamp );

            if( state == State::Playing ) {
                // Invoke libretro core
                symbols.retro_run();
            }

            break;
        }

        // Eat this command, it came from us
        case Command::CoreFPS: {
            break;
        }

        case Command::SetSource: {
            qCDebug( phxCore ) << command;
            emit commandOut( command, data, timeStamp );

            QMap<QString, QVariant> map = data.toMap();
            QStringMap stringMap;

            for( QString key : map.keys() ) {
                stringMap[ key ] = map[ key ].toString();
            }

            this->source = stringMap;
            break;
        }

        case Command::ControllerRemoved: {
            int instanceID = data.toInt();
            gamepads.remove( instanceID );
            emit commandOut( command, data, timeStamp );
        }

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}

void LibretroCore::dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    emit dataOut( type, mutex, data, bytes, timeStamp );

    switch( type ) {
        // Make a copy of the data into our own gamepad list
        case DataType::Input: {
            mutex->lock();
            Gamepad gamepad = *( Gamepad * )data;
            int instanceID = gamepad.instanceID;
            gamepads[ instanceID ] = gamepad;
            mutex->unlock();
            break;
        }

        case DataType::TouchInput: {
            break;
        }

        default:
            break;
    }
}

// Protected

LibretroCore *LibretroCore::core = nullptr;

void LibretroCore::emitAudioData( void *data, size_t bytes ) {
    emit dataOut( DataType::Audio, &mutex, data, bytes, QDateTime::currentMSecsSinceEpoch() );
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

        //emit producerFormat( producerFmt );
        QVariant variant;
        variant.setValue( producerFmt );
        emit commandOut( Command::VideoFormat, variant, QDateTime::currentMSecsSinceEpoch() );
    }

    emit dataOut( DataType::Video, &mutex, data, bytes, QDateTime::currentMSecsSinceEpoch() );
}

// Private

void LibretroCore::LibretroCore::loadSaveData() {
    saveDataBuf = symbols.retro_get_memory_data( RETRO_MEMORY_SAVE_RAM );

    if( !saveDataBuf ) {
        qCInfo( phxCore ) << "The core will handle saving (passed a null pointer when asked for save buffer)";
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

void LibretroCore::getAVInfo( retro_system_av_info *avInfo ) {
    // Audio

    // The Libretro API only support 16-bit stereo PCM
    audioSampleRate = avInfo->timing.sample_rate;
    emit commandOut( Command::SampleRate, audioSampleRate, QDateTime::currentMSecsSinceEpoch() );

    // Video

    producerFmt.videoAspectRatio = avInfo->geometry.aspect_ratio <= 0.0 ?
                                   ( qreal )avInfo->geometry.base_width / avInfo->geometry.base_height :
                                   avInfo->geometry.aspect_ratio;
    producerFmt.videoBytesPerPixel = QImage().toPixelFormat( producerFmt.videoPixelFormat ).bitsPerPixel() / 8;
    producerFmt.videoBytesPerLine = avInfo->geometry.base_width * producerFmt.videoBytesPerPixel;
    producerFmt.videoFramerate = avInfo->timing.fps;

    producerFmt.videoSize.setWidth( avInfo->geometry.base_width );
    producerFmt.videoSize.setHeight( avInfo->geometry.base_height );

    qCDebug( phxCore ) << "Core claims an aspect ratio of:" << avInfo->geometry.aspect_ratio;
    qCDebug( phxCore ) << "Using aspect ratio:" << producerFmt.videoAspectRatio;
    qCDebug( phxCore ) << "Base video size:" << QSize( avInfo->geometry.base_width, avInfo->geometry.base_height );
    qCDebug( phxCore ) << "Maximum video size:" << QSize( avInfo->geometry.max_width, avInfo->geometry.max_height );

    QVariant variant;
    variant.setValue( producerFmt );
    emit commandOut( Command::VideoFormat, variant, QDateTime::currentMSecsSinceEpoch() );
}

void LibretroCore::allocateBufferPool( retro_system_av_info *avInfo ) {
    // Allocate buffers to fit this max size
    // Assume 16-bit stereo audio, 32-bit video
    for( int i = 0; i < POOL_SIZE; i++ ) {
        // Allocate a bit extra as some cores' numbers do not add up...
        audioBufferPool[ i ] = new int16_t[( size_t )( avInfo->timing.sample_rate * 3 ) ]();
        videoBufferPool[ i ] = new quint8[( size_t )( avInfo->geometry.max_width * avInfo->geometry.max_height * 4 ) ]();
    }
}

void LibretroCore::audioSampleCallback( int16_t left, int16_t right ) {
    LibretroCore *core = LibretroCore::core;

    // Sanity check
    Q_ASSERT_X( core->audioBufferCurrentByte < core->audioSampleRate * 5,
                "audio batch callback", QString( "Buffer pool overflow (%1)" ).arg( core->audioBufferCurrentByte ).toLocal8Bit() );

    // Stereo audio is interleaved, left then right
    core->mutex.lock();
    core->audioBufferPool[ core->audioPoolCurrentBuffer ][ core->audioBufferCurrentByte / 2 ] = left;
    core->audioBufferPool[ core->audioPoolCurrentBuffer ][ core->audioBufferCurrentByte / 2 + 1 ] = right;
    core->mutex.unlock();

    // Each frame is 4 bytes (16-bit stereo)
    core->audioBufferCurrentByte += 4;

    // Flush if we have more than 1/2 of a frame's worth of data
    if( core->audioBufferCurrentByte > core->audioSampleRate * 4 / core->producerFmt.videoFramerate / 2 ) {
        core->emitAudioData( core->audioBufferPool[ core->audioPoolCurrentBuffer ], core->audioBufferCurrentByte );
        core->audioBufferCurrentByte = 0;
        core->audioPoolCurrentBuffer = ( core->audioPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }
}

size_t LibretroCore::audioSampleBatchCallback( const int16_t *data, size_t frames ) {
    LibretroCore *core = LibretroCore::core;

    // Sanity check
    Q_ASSERT_X( core->audioBufferCurrentByte < core->audioSampleRate * 5,
                "audio batch callback",
                QString( "Buffer pool overflow (%1)" ).arg( core->audioBufferCurrentByte ).toLocal8Bit() );

    // Need to do a bit of pointer arithmetic to get the right offset (the buffer is indexed in increments of shorts -- 2 bytes)
    int16_t *dst_init = core->audioBufferPool[ core->audioPoolCurrentBuffer ];
    int16_t *dst = dst_init + ( core->audioBufferCurrentByte / 2 );

    // Copy the incoming data
    core->mutex.lock();
    memcpy( dst, data, frames * 4 );
    core->mutex.unlock();

    // Each frame is 4 bytes (16-bit stereo)
    core->audioBufferCurrentByte += frames * 4;

    // Flush if we have 1/2 of a frame's worth of data or more
    if( core->audioBufferCurrentByte >= core->audioSampleRate * 4 / core->producerFmt.videoFramerate / 2 ) {
        core->emitAudioData( core->audioBufferPool[ core->audioPoolCurrentBuffer ], core->audioBufferCurrentByte );
        core->audioBufferCurrentByte = 0;
        core->audioPoolCurrentBuffer = ( core->audioPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }

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
                //qCDebug( phxCore ) << "\t\t" << key << descriptor->description;
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
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE (15)(handled)";
            auto *retroVariable = static_cast<struct retro_variable *>( data );

            if( core->variables.contains( retroVariable->key ) ) {
                const auto &var = core->variables[ retroVariable->key ];

                if( var.isValid() ) {
                    retroVariable->value = var.value().c_str();

                    if( strlen( var.value().c_str() ) ) {
                        //qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE (15)(handled)" << var.key().c_str() << var.value().c_str();
                        return true;
                    }
                }
            }

            break;
        }

        case RETRO_ENVIRONMENT_SET_VARIABLES: { // 16
            qCDebug( phxCore ) << "RETRO_ENVIRONMENT_SET_VARIABLES (16) (handled)";
            auto *rv = ( const struct retro_variable * )( data );

            for( ; rv->key != NULL; rv++ ) {
                LibretroVariable v( rv );

                if( !( core->variables.contains( v.key() ) ) ) {
                    core->variables.insert( v.key(), v );
                }

                //qCDebug( phxCore ) << "        " << v;
            }

            return true;
            break;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: { // 17
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE_UPDATE (17)(handled)";
            // Let the core know we have some variable changes if we set our internal flag (clear it so the change only happens once)
            // TODO: Protect all variable-touching code with mutexes?
            if( core->variablesHaveChanged ) {
                // Special case: Force DeSmuME's pointer type variable to "touch" in order to work with our touch code
                if( core->variables.contains( "desmume_pointer_type" ) ) {
                    core->variables[ "desmume_pointer_type" ].setValue( "touch" );
                }

                core->variablesHaveChanged = false;
                *( bool * )data = true;
                return true;
            } else {
                *( bool * )data = false;
                return false;
            }

            break;
        }

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
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SAVE_DIRECTORY (31) (handled)";
            *( const char ** )data = core->savePathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: { // 32
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_systemAVInfo (32) (handled)";
            return true;
        }
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

        case RETRO_ENVIRONMENT_SET_GEOMETRY: { // 37
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_GEOMETRY (37) (handled)";

            // Get info from the core
            retro_system_av_info *avInfo = new retro_system_av_info();
            core->symbols.retro_get_system_av_info( avInfo );
            // Although we hope the core would have updated its internal av_info struct by now, we'll play it safe and
            // use the given geometry
            memcpy( &( avInfo->geometry ), data, sizeof( struct retro_game_geometry ) );
            core->getAVInfo( avInfo );
            delete avInfo;

            return true;
        }

        case RETRO_ENVIRONMENT_GET_USERNAME: // 38
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_USERNAME (38)";
            break;

        case RETRO_ENVIRONMENT_GET_LANGUAGE: // 39
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LANGUAGE (39)";
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
    // Q_UNUSED( index )

    // Touch input
    if( device == RETRO_DEVICE_POINTER && port == 0 && index == 0 ) {
        switch( id ) {
            // 0 or 1
            case RETRO_DEVICE_ID_POINTER_PRESSED: {
                return core->touchState ? 1 : 0;
            }
            break;

            // -0x7FFF to 0x7FFF, clamp incoming values to [0.0, 1.0]
            // -32767 to 32767... odd range to use IMO (literally)
            case RETRO_DEVICE_ID_POINTER_X: {
                qreal x = 1.0 + -core->touchCoords.x();

                if( x > 1.0 ) {
                    x = 1.0;
                }

                if( x < 0.0 ) {
                    x = 0.0;
                }

                int ret = 0xFFFE;
                ret *= x;
                ret -= 0x7FFF;
                ret &= 0xFFFF;
                return ret;
            }
            break;

            case RETRO_DEVICE_ID_POINTER_Y: {
                qreal y = 1.0 + -core->touchCoords.y();

                if( y > 1.0 ) {
                    y = 1.0;
                }

                if( y < 0.0 ) {
                    y = 0.0;
                }

                int ret = 0xFFFE;
                ret *= y;
                ret -= 0x7FFF;
                ret &= 0xFFFF;
                return ret;
            }
            break;

            default:
                break;
        }

    }

    // TODO: Support more than the default joypad
    if( device != RETRO_DEVICE_JOYPAD ) {
        return 0;
    }

    // TODO: ...and their buttons
    if( id > 15 ) {
        return 0;
    }

    int16_t value = 0;

    // TODO: Don't OR all controllers together!
    for( Gamepad gamepad : core->gamepads ) {
        if( gamepad.button[ SDL_CONTROLLER_BUTTON_A ] && id == RETRO_DEVICE_ID_JOYPAD_A ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_B ] && id == RETRO_DEVICE_ID_JOYPAD_B ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_X ] && id == RETRO_DEVICE_ID_JOYPAD_X ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_Y ] && id == RETRO_DEVICE_ID_JOYPAD_Y ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_LEFTSHOULDER ] && id == RETRO_DEVICE_ID_JOYPAD_L ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER ] && id == RETRO_DEVICE_ID_JOYPAD_R ) {
            value |= 1;
        }

        // TODO: L2? R2?

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_LEFTSTICK ] && id == RETRO_DEVICE_ID_JOYPAD_L3 ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_RIGHTSTICK ] && id == RETRO_DEVICE_ID_JOYPAD_R3 ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_DPAD_UP ] && id == RETRO_DEVICE_ID_JOYPAD_UP ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_DPAD_DOWN ] && id == RETRO_DEVICE_ID_JOYPAD_DOWN ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_DPAD_LEFT ] && id == RETRO_DEVICE_ID_JOYPAD_LEFT ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_DPAD_RIGHT ] && id == RETRO_DEVICE_ID_JOYPAD_RIGHT ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_BACK ] && id == RETRO_DEVICE_ID_JOYPAD_SELECT ) {
            value |= 1;
        }

        else if( gamepad.button[ SDL_CONTROLLER_BUTTON_START ] && id == RETRO_DEVICE_ID_JOYPAD_START ) {
            value |= 1;
        }
    }

    return value;
}

void LibretroCore::videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch ) {
    Q_UNUSED( width );

    // Current frame exists, send it on its way
    if( data ) {

        core->mutex.lock();
        memcpy( core->videoBufferPool[ core->videoPoolCurrentBuffer ], data, height * pitch );
        core->mutex.unlock();

        core->emitVideoData( core->videoBufferPool[ core->videoPoolCurrentBuffer ], width, height, pitch, pitch * height );
        core->videoPoolCurrentBuffer = ( core->videoPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }

    // Current frame is a dupe, send the last actual frame again
    else {
        core->emitVideoData( core->videoBufferPool[ core->videoPoolCurrentBuffer ], width, height, pitch, pitch * height );
    }

    return;
}

QString LibretroCore::inputTupleToString( unsigned port, unsigned device, unsigned index, unsigned id ) {
    return QString::number( port, 16 ) % ',' % QString::number( device, 16 ) % ','
           % QString::number( index, 16 ) % ',' % QString::number( id, 16 );
}
