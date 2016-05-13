#include "libretrocore.h"
#include "SDL.h"
#include "SDL_gamecontroller.h"

#include <QString>
#include <QStringBuilder>
#include <QThread>

LibretroCore::LibretroCore( Core *parent ): Core( parent ),
    // Private
    systemInfo( new retro_system_info ) {

    // All Libretro cores are pausable, just stop calling retro_run()
    pausable = true;
}

LibretroCore::~LibretroCore() {
    for( int i = 0; i < POOL_SIZE; i++ ) {
        delete audioBufferPool[i];
        delete videoBufferPool[i];
    }

    delete systemInfo;
}

// Public

void LibretroCore::getAVInfo( retro_system_av_info *avInfo ) {
    // Audio

    // The Libretro API only support 16-bit stereo PCM
    core.audioSampleRate = avInfo->timing.sample_rate;
    emit commandOut( Node::Command::SampleRate, core.audioSampleRate, QDateTime::currentMSecsSinceEpoch() );

    // Video

    core.producerFmt.videoAspectRatio = avInfo->geometry.aspect_ratio <= 0.0 ?
                                        ( qreal )avInfo->geometry.base_width / avInfo->geometry.base_height :
                                        avInfo->geometry.aspect_ratio;
    core.producerFmt.videoBytesPerPixel = QImage().toPixelFormat( core.producerFmt.videoPixelFormat ).bitsPerPixel() / 8;
    core.producerFmt.videoBytesPerLine = avInfo->geometry.base_width * core.producerFmt.videoBytesPerPixel;
    core.producerFmt.videoFramerate = avInfo->timing.fps;

    core.producerFmt.videoSize.setWidth( avInfo->geometry.base_width );
    core.producerFmt.videoSize.setHeight( avInfo->geometry.base_height );

    qCDebug( phxCore ) << "Core claims an aspect ratio of:" << avInfo->geometry.aspect_ratio;
    qCDebug( phxCore ) << "Using aspect ratio:" << core.producerFmt.videoAspectRatio;
    qCDebug( phxCore ) << "Base video size:" << QSize( avInfo->geometry.base_width, avInfo->geometry.base_height );
    qCDebug( phxCore ) << "Maximum video size:" << QSize( avInfo->geometry.max_width, avInfo->geometry.max_height );

    QVariant variant;
    variant.setValue( core.producerFmt );
    emit commandOut( Node::Command::VideoFormat, variant, QDateTime::currentMSecsSinceEpoch() );
}

void LibretroCore::emitAudioData( void *data, size_t bytes ) {
    emit dataOut( Node::DataType::Audio, &core.mutex, data, bytes, QDateTime::currentMSecsSinceEpoch() );
}

void LibretroCore::emitVideoData( void *data, unsigned width, unsigned height, size_t pitch, size_t bytes ) {
    // Cores can change the size of the video they output (within the bounds they set on load) at any time
    if( core.producerFmt.videoSize != QSize( width, height ) || core.producerFmt.videoBytesPerLine != pitch ) {
        qCDebug( phxCore ) << "Video resized!";
        qCDebug( phxCore ) << "Old video size:" << core.producerFmt.videoSize;
        qCDebug( phxCore ) << "New video size:" << QSize( width, height );

        core.producerFmt.videoBytesPerLine = pitch;
        core.producerFmt.videoSize.setWidth( width );
        core.producerFmt.videoSize.setHeight( height );

        QVariant variant;
        variant.setValue( core.producerFmt );
        emit commandOut( Node::Command::VideoFormat, variant, QDateTime::currentMSecsSinceEpoch() );
    }

    emit dataOut( Node::DataType::Video, &core.mutex, data, bytes, QDateTime::currentMSecsSinceEpoch() );
}

void LibretroCore::updateVariables() {
    variablesAreDirty = true;
}

// Global
LibretroCore core;

void loadSaveData() {
    core.saveDataBuf = ( core.symbols.retro_get_memory_data )( RETRO_MEMORY_SAVE_RAM );

    if( !core.saveDataBuf ) {
        qCInfo( phxCore ) << "The core will handle saving (passed a null pointer when asked for save buffer)";
        return;
    }

    QFile file( core.savePathInfo.absolutePath() % QStringLiteral( "/" ) %
                core.gameFileInfo.baseName() % QStringLiteral( ".sav" ) );

    if( file.open( QIODevice::ReadOnly ) ) {
        QByteArray data = file.readAll();
        memcpy( core.saveDataBuf, data.data(), data.size() );

        qCDebug( phxCore ) << Q_FUNC_INFO << file.fileName() << "(true)";
        file.close();
    }

    else {
        qCDebug( phxCore ) << Q_FUNC_INFO << file.fileName() << "(false)";
    }
}

void storeSaveData() {
    QString localFile = core.savePathInfo.absolutePath() % QStringLiteral( "/" ) %
                        core.gameFileInfo.baseName() % QStringLiteral( ".sav" );

    qCDebug( phxCore ).nospace() << "Saving: " << localFile;

    if( core.saveDataBuf == nullptr ) {
        qCDebug( phxCore ).nospace() << "Skipping, the core has done the save by itself";
        return;
    }

    QFile file( localFile );

    if( file.open( QIODevice::WriteOnly ) ) {
        char *data = static_cast<char *>( core.saveDataBuf );
        size_t size = core.symbols.retro_get_memory_size( RETRO_MEMORY_SAVE_RAM );
        file.write( data, size );
        file.close();
        qCDebug( phxCore ) << "Save successful";
    }

    else {
        qCDebug( phxCore ).nospace() << "Failed: " << file.errorString();
    }
}

void allocateBufferPool( retro_system_av_info *avInfo ) {
    // Allocate buffers to fit this max size
    // Assume 16-bit stereo audio, 32-bit video
    for( int i = 0; i < POOL_SIZE; i++ ) {
        // Allocate a bit extra as some cores' numbers do not add up...
        core.audioBufferPool[ i ] = new int16_t[( size_t )( avInfo->timing.sample_rate * 3 ) ]();
        core.videoBufferPool[ i ] = new quint8[( size_t )( avInfo->geometry.max_width * avInfo->geometry.max_height * 4 ) ]();
    }
}

void audioSampleCallback( int16_t left, int16_t right ) {
    // Sanity check
    Q_ASSERT_X( core.audioBufferCurrentByte < core.audioSampleRate * 5,
                "audio batch callback", QString( "Buffer pool overflow (%1)" ).arg( core.audioBufferCurrentByte ).toLocal8Bit() );

    // Stereo audio is interleaved, left then right
    core.mutex.lock();
    core.audioBufferPool[ core.audioPoolCurrentBuffer ][ core.audioBufferCurrentByte / 2 ] = left;
    core.audioBufferPool[ core.audioPoolCurrentBuffer ][ core.audioBufferCurrentByte / 2 + 1 ] = right;
    core.mutex.unlock();

    // Each frame is 4 bytes (16-bit stereo)
    core.audioBufferCurrentByte += 4;

    // Flush if we have more than 1/2 of a frame's worth of data
    if( core.audioBufferCurrentByte > core.audioSampleRate * 4 / core.producerFmt.videoFramerate / 2 ) {
        core.emitAudioData( core.audioBufferPool[ core.audioPoolCurrentBuffer ], core.audioBufferCurrentByte );
        core.audioBufferCurrentByte = 0;
        core.audioPoolCurrentBuffer = ( core.audioPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }
}

size_t audioSampleBatchCallback( const int16_t *data, size_t frames ) {
    // Sanity check
    Q_ASSERT_X( core.audioBufferCurrentByte < core.audioSampleRate * 5,
                "audio batch callback",
                QString( "Buffer pool overflow (%1)" ).arg( core.audioBufferCurrentByte ).toLocal8Bit() );

    // Need to do a bit of pointer arithmetic to get the right offset (the buffer is indexed in increments of shorts -- 2 bytes)
    int16_t *dst_init = core.audioBufferPool[ core.audioPoolCurrentBuffer ];
    int16_t *dst = dst_init + ( core.audioBufferCurrentByte / 2 );

    // Copy the incoming data
    core.mutex.lock();
    memcpy( dst, data, frames * 4 );
    core.mutex.unlock();

    // Each frame is 4 bytes (16-bit stereo)
    core.audioBufferCurrentByte += frames * 4;

    // Flush if we have 1/2 of a frame's worth of data or more
    if( core.audioBufferCurrentByte >= core.audioSampleRate * 4 / core.producerFmt.videoFramerate / 2 ) {
        core.emitAudioData( core.audioBufferPool[ core.audioPoolCurrentBuffer ], core.audioBufferCurrentByte );
        core.audioBufferCurrentByte = 0;
        core.audioPoolCurrentBuffer = ( core.audioPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }

    return frames;
}

bool environmentCallback( unsigned cmd, void *data ) {

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
            *( const char ** )data = core.systemPathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: { // 10
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PIXEL_FORMAT (10) (handled)";

            retro_pixel_format *pixelformat = ( enum retro_pixel_format * )data;

            switch( *pixelformat ) {
                case RETRO_PIXEL_FORMAT_0RGB1555:
                    core.producerFmt.videoPixelFormat = QImage::Format_RGB555;
                    qCDebug( phxCore ) << "\t\tPixel format: 0RGB1555 aka QImage::Format_RGB555";
                    return true;

                case RETRO_PIXEL_FORMAT_RGB565:
                    core.producerFmt.videoPixelFormat = QImage::Format_RGB16;
                    qCDebug( phxCore ) << "\t\tPixel format: RGB565 aka QImage::Format_RGB16";
                    return true;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    core.producerFmt.videoPixelFormat = QImage::Format_RGB32;
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
                QString key = inputTupleToString( descriptor->port, descriptor->device, descriptor->index, descriptor->id );
                core.inputDescriptors[ key ] = QString( descriptor->description );
                //qCDebug( phxCore ) << "\t\t" << key << descriptor->description;
            }

            return true;
        }

        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: { // 12
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK (12) (handled)";
            core.symbols.retro_keyboard_event = ( decltype( LibretroSymbols::retro_keyboard_event ) )data;
            break;
        }

        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: // 13
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE (13)";
            break;

        case RETRO_ENVIRONMENT_SET_HW_RENDER: // 14
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_HW_RENDER (14) (handled)";

            core.producerFmt.videoMode = HARDWARERENDER;

            core.openGLContext = *( retro_hw_render_callback * )data;

            switch( core.openGLContext.context_type ) {
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
                    qCritical() << "\t\tRETRO_HW_CONTEXT: " << core.openGLContext.context_type << " was not handled!";
                    break;
            }

            break;

        case RETRO_ENVIRONMENT_GET_VARIABLE: { // 15
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE (15)(handled)";
            auto *retroVariable = static_cast<struct retro_variable *>( data );

            if( core.variables.contains( retroVariable->key ) ) {
                const auto &var = core.variables[ retroVariable->key ];

                if( var.isValid() ) {
                    retroVariable->value = var.value().data();

                    if( !var.value().isEmpty() ) {
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

                if( !( core.variables.contains( v.key() ) ) ) {
                    core.variables.insert( v.key(), v );
                }

            }

            return true;
            break;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: { // 17
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE_UPDATE (17)(handled)";
            // Let the core know we have some variable changes if we set our internal flag (clear it so the change only happens once)
            // TODO: Protect all variable-touching code with mutexes?
            if( core.variablesAreDirty ) {
                // Special case: Force DeSmuME's pointer type variable to "touch" in order to work with our touch code
                if( core.variables.contains( "desmume_pointer_type" ) ) {
                    core.variables[ "desmume_pointer_type" ].setValue( QByteArrayLiteral( "touch" ) );
                }

                core.variablesAreDirty = false;
                *static_cast<bool *>( data ) = true;
                return true;
            } else {
                *static_cast<bool *>( data ) = false;
                return false;
            }


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
            *( const char ** )data = core.corePathCString;
            break;
        }

        // 20 has been deprecated

        case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: // 21
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK (21) (handled)";
            core.symbols.retro_frame_time = ( decltype( LibretroSymbols::retro_frame_time ) )data;
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
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CAMERA_INTERFACE (RETRO_ENVIROxNMENT_EXPERIMENTAL)(26)";
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
            *( const char ** )data = core.systemPathCString;
            return true;
        }
        break;

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: { // 31
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SAVE_DIRECTORY (31) (handled)";
            *( const char ** )data = core.savePathCString;
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
            core.symbols.retro_get_system_av_info( avInfo );
            // Although we hope the core would have updated its internal av_info struct by now, we'll play it safe and
            // use the given geometry
            memcpy( &( avInfo->geometry ), data, sizeof( struct retro_game_geometry ) );
            core.getAVInfo( avInfo );
            delete avInfo;

            return true;
        }

        case RETRO_ENVIRONMENT_GET_USERNAME: // 38
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_USERNAME (38)";
            break;

        case RETRO_ENVIRONMENT_GET_LANGUAGE: // 39
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LANGUAGE (39)";
            break;

        case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER: { // 40
            //qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER (40)";
            return false;
            break;
        }

        default:
            qCDebug( phxCore ) << "Error: Environment command " << cmd << " is not defined in the frontend's libretro.h!.";
            return false;

    }

    // Command was not handled
    return false;

}

void inputPollCallback( void ) {
    // Do nothing, this is handled before retro_run() is called
    return;
}

void logCallback( enum retro_log_level level, const char *fmt, ... ) {
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

int16_t inputStateCallback( unsigned port, unsigned device, unsigned index, unsigned id ) {
    // Q_UNUSED( index )

    // Touch input
    if( device == RETRO_DEVICE_POINTER && port == 0 && index == 0 ) {

        // Information taken directly from VideoOutput's QML for aspect ratio correction
        // FIXME: Don't assume VideoOutput fills entire window
        // TODO: Do all this upstream? Maybe in PhoenixWindow?

        qreal windowWidth = core.geometry.width();
        qreal windowHeight = core.geometry.height();
        qreal aspectRatio = core.producerFmt.videoAspectRatio;

        qreal letterBoxHeight = windowWidth / aspectRatio;
        qreal letterBoxWidth = windowWidth;
        qreal pillBoxWidth = windowHeight * aspectRatio;
        qreal pillBoxHeight = windowHeight;
        bool pillBoxing = ( windowWidth / windowHeight / aspectRatio ) > 1.0;

        // Fit mode (0): Maintain aspect ratio, fit all content within window, letterboxing/pillboxing as necessary
        qreal fitModeWidth = pillBoxing ? pillBoxWidth : letterBoxWidth;
        qreal fitModeHeight = pillBoxing ? pillBoxHeight : letterBoxHeight;

        // Stretch mode (1): Fit to parent, ignore aspect ratio
        //qreal stretchModeWidth = windowWidth;
        //qreal stretchModeHeight = windowHeight;

        // Fill mode (2): Maintian aspect ratio, fill window with content, cropping the remaining stuff
        // TODO
        //qreal fillModeWidth = 0;
        //qreal fillModeHeight = 0;

        // Center mode (3): Show at core's native resolution
        // TODO
        //qreal centerModeWidth = 0;
        //qreal centerModeHeight = 0;

        switch( id ) {
            // 0 or 1
            case RETRO_DEVICE_ID_POINTER_PRESSED: {
                return ( core.mouse.buttons & Qt::LeftButton ) ? 1 : 0;
            }
            break;

            case RETRO_DEVICE_ID_POINTER_X: {
                qreal windowX = core.mouse.position.x();
                qreal x = windowX / core.geometry.width();

                // Aspect ratio corrections
                // Mode 1 requires no correction
                // TODO: 2 and 3
                if( core.aspectMode == 0 ) {
                    // Center it
                    qreal centerX = ( windowWidth / 2 ) - ( fitModeWidth / 2 );
                    qreal center = centerX / windowWidth;
                    x -= center;

                    // Scale it
                    qreal scale = windowWidth / fitModeWidth;
                    x *= scale;
                }

                // Clamp to [0.0, 1.0]
                if( x > 1.0 ) {
                    x = 1.0;
                } else if( x < 0.0 ) {
                    x = 0.0;
                }

                // Map to [-32767, 32767]
                int ret = ( 32767 * 2 ) + 1;
                ret *= x;
                ret -= 32767;
                return ret;
            }
            break;

            case RETRO_DEVICE_ID_POINTER_Y: {
                qreal windowY = core.mouse.position.y();
                qreal y = windowY / core.geometry.height();

                // Aspect ratio corrections
                // Mode 1 requires no correction
                // TODO: 2 and 3
                if( core.aspectMode == 0 ) {
                    // Center it
                    qreal centerY = ( windowHeight / 2 ) - ( fitModeHeight / 2 );
                    qreal center = centerY / windowHeight;
                    y -= center;

                    // Scale it
                    qreal scale = windowHeight / fitModeHeight;
                    y *= scale;
                }

                // Clamp to [0.0, 1.0]
                if( y > 1.0 ) {
                    y = 1.0;
                } else if( y < 0.0 ) {
                    y = 0.0;
                }

                // Map to [-32767, 32767]
                int ret = ( 32767 * 2 ) + 1;
                ret *= y;
                ret -= 32767;
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
    for( GamepadState gamepad : core.gamepads ) {
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

void videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch ) {
    Q_UNUSED( width );

    // Current frame exists, send it on its way
    if( data ) {
        core.mutex.lock();
        memcpy( core.videoBufferPool[ core.videoPoolCurrentBuffer ], data, height * pitch );
        core.mutex.unlock();

        core.emitVideoData( core.videoBufferPool[ core.videoPoolCurrentBuffer ], width, height, pitch, pitch * height );
        core.videoPoolCurrentBuffer = ( core.videoPoolCurrentBuffer + 1 ) % POOL_SIZE;
    }

    // Current frame is a dupe, send the last actual frame again
    else {
        core.emitVideoData( core.videoBufferPool[ core.videoPoolCurrentBuffer ], width, height, pitch, pitch * height );
    }

    return;
}

QString inputTupleToString( unsigned port, unsigned device, unsigned index, unsigned id ) {
    return QString::number( port, 16 ) % ',' % QString::number( device, 16 ) % ','
           % QString::number( index, 16 ) % ',' % QString::number( id, 16 );
}
