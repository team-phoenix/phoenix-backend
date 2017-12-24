#include "retrocallbacks.h"

#include "emulator.h"
#include "logging.h"
#include "gamepad.h"


static AbstractEmulator *emulator = nullptr;

void RetroCallbacks::setEmulator( AbstractEmulator *emu ) {
    emulator = emu;
}

void RetroCallbacks::audioSampleCallback(int16_t t_left, int16_t t_right) {
    emulator->routeAudioSample( t_left, t_right );
}

size_t RetroCallbacks::audioSampleBatchCallback(const int16_t *t_data, size_t t_frames) {
    emulator->routeAudioBatch( t_data, t_frames );
}

bool RetroCallbacks::environmentCallback(unsigned cmd, void *data) {

    switch( cmd ) {
        case RETRO_ENVIRONMENT_SET_ROTATION: // 1
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_ROTATION (1)";
            break;

        case RETRO_ENVIRONMENT_GET_OVERSCAN: {// 2
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_OVERSCAN (2) (handled)";
            // Crop away overscan
            return true;
        }

        case RETRO_ENVIRONMENT_GET_CAN_DUPE: { // 3
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CAN_DUPE (3) (handled)";
            *( static_cast<bool *>( data ) ) = true;
            return true;
        }

            // 4 and 5 have been deprecated

        case RETRO_ENVIRONMENT_SET_MESSAGE: {// 6
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_MESSAGE (6)";
            retro_message *message = static_cast<retro_message *>( data );
            (void)message;

            break;
        }

        case RETRO_ENVIRONMENT_SHUTDOWN: // 7
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SHUTDOWN (7)";
            break;

        case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL: // 8
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL (8)";
            break;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: { // 9
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY (9) (handled)";
            //*( const char ** )data = libretroCore.systemPathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: { // 10
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PIXEL_FORMAT (10) (handled)";

            retro_pixel_format *pixelformat = static_cast<enum retro_pixel_format *> ( data );


            switch( *pixelformat ) {
                case RETRO_PIXEL_FORMAT_0RGB1555:
                    emulator->m_pixelFormat = QImage::Format_RGB555;

//                    emulator->sendProcessMessage( {
//                                                                    { "retro_cmd", {
//                                                                          "environ_cb",
//                                                                      }
//                                                                }
//                                                                });
                    qCDebug( phxCore ) << "\t\tPixel format: 0RGB1555 aka QImage::Format_RGB555";
                    return true;

                case RETRO_PIXEL_FORMAT_RGB565:
                    emulator->m_pixelFormat  = QImage::Format_RGB16;
                    qCDebug( phxCore ) << "\t\tPixel format: RGB565 aka QImage::Format_RGB16";
                    return true;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    emulator->m_pixelFormat  = QImage::Format_RGB32;
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

//            for( retro_input_descriptor *descriptor = ( retro_input_descriptor * )data; descriptor->description; descriptor++ ) {
//                QString key = LibretroCoreInputTupleToString( descriptor->port, descriptor->device, descriptor->index, descriptor->id );
//                libretroCore.inputDescriptors[ key ] = QString( descriptor->description );
//                //qCDebug( phxCore ) << "\t\t" << key << descriptor->description;
//            }

            return true;
        }

        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: { // 12
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK (12) (handled)";
            emulator->m_libretroLibrary.retro_keyboard_event = ( decltype( LibretroLibrary::retro_keyboard_event ) )data;
            break;
        }

        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: // 13
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE (13)";
            break;

        case RETRO_ENVIRONMENT_SET_HW_RENDER: { // 14
            qDebug() << "\tRETRO_ENVIRONMENT_SET_HW_RENDER (14) (handled)";

//            libretroCore.videoFormat.videoMode = HARDWARERENDER;

//            retro_hw_render_callback *hardwareRenderData = static_cast<retro_hw_render_callback *>( data );

//            hardwareRenderData->get_current_framebuffer = getFramebufferCallback;
//            hardwareRenderData->get_proc_address = openGLProcAddressCallback;

//            emulator->m_symbols.retro_hw_context_reset = hardwareRenderData->context_reset;

//            QSurfaceFormat surfaceFmt;

//            switch( hardwareRenderData->context_type ) {
//                case RETRO_HW_CONTEXT_NONE:
//                    qWarning() << "\t\tNo hardware context was selected";
//                    break;

//                case RETRO_HW_CONTEXT_OPENGL: {
//                    if( emulator->m_offscreenSurface->format().majorVersion() < 2
//                            || emulator->m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGL ) {
//                        qWarning() << "\t\tOpenGL 2.x not available! Please install a driver for your GPU that supports modern OpenGL";
//                        return false;
//                    }


//                    qDebug() << "\t\tOpenGL 2 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES2: {
//                    if( emulator->m_offscreenSurface->format().majorVersion() < 2
//                            || emulator->m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGLES ) {
//                        qWarning() << "\t\tOpenGL ES 2.0 not available! Please install a driver for your GPU that supports this";
//                        return false;
//                    }

//                    qDebug() << "\t\tOpenGL ES 2.0 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGL_CORE: {
//                    if( emulator->m_offscreenSurface->format().renderableType() == QSurfaceFormat::OpenGL ) {
//                        if( emulator->m_offscreenSurface->format().majorVersion() > static_cast<int>( hardwareRenderData->version_major ) ) {
//                            qDebug().nospace() << "\t\tOpenGL " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }

//                        if( emulator->m_offscreenSurface->format().majorVersion() >= static_cast<int>( hardwareRenderData->version_major ) &&
//                            emulator->m_offscreenSurface->format().minorVersion() >= static_cast<int>( hardwareRenderData->version_minor ) ) {
//                            qDebug().nospace() << "\t\tOpenGL " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }
//                    }

//                    qWarning().nospace() << "\t\tOpenGL " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                         << "not available! Please install a driver for your GPU that supports this version or consider upgrading";
//                    return false;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES3: {
//                    if( emulator->m_offscreenSurface->format().majorVersion() < 3
//                            || emulator->m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGLES ) {
//                        qWarning() << "\t\tOpenGL ES 3.0 not available! Please install a driver for your GPU that supports this";
//                        return false;
//                    }

//                    qDebug() << "\t\tOpenGL ES 3.0 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES_VERSION: {
//                    if( emulator->m_offscreenSurface->format().renderableType() == QSurfaceFormat::OpenGLES ) {
//                        if( emulator->m_offscreenSurface->format().majorVersion() > static_cast<int>( hardwareRenderData->version_major ) ) {
//                            qDebug().nospace() << "\t\tOpenGL ES " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }

//                        if( emulator->m_offscreenSurface->format().majorVersion() >= static_cast<int>( hardwareRenderData->version_major ) &&
//                            emulator->m_offscreenSurface->format().minorVersion() >= static_cast<int>( hardwareRenderData->version_minor ) ) {
//                            qDebug().nospace() << "\t\tOpenGL ES " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }
//                    }

//                    qWarning().nospace() << "\t\tOpenGL ES " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                         << "not available! Please install a driver for your GPU that supports this version or consider upgrading";
//                    return false;
//                }

//                case RETRO_HW_CONTEXT_VULKAN:
//                    qWarning() << "\t\tUnsupported Vulkan context selected";
//                    return false;

//                default:
//                    qCritical() << "\t\tRETRO_HW_CONTEXT: " << hardwareRenderData->context_type << " was not handled!";
//                    break;
//            }

            break;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE: { // 15
            //qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE (15)(handled)";
            retro_variable *retroVariable = static_cast<retro_variable *>( data );
            retroVariable->value = nullptr;

            if ( emulator->m_variableModel.contains( *retroVariable ) ) {

                retroVariable->value = emulator->m_variableModel.currentValue( retroVariable->key ).constData();

                return true;
            }

            return false;
        }

        case RETRO_ENVIRONMENT_SET_VARIABLES: { // 16
            qCDebug( phxCore ) << "RETRO_ENVIRONMENT_SET_VARIABLES (16) (handled)";
            const retro_variable *variable = ( const retro_variable * )( data );

            for ( ; variable->key != nullptr; variable++ ) {
                qDebug( phxCore ) << "\tretro_variable:" << variable->key << variable->value;
                if ( variable->key != nullptr && variable->key == "" ) {
                    emulator->m_variableModel.insert( *variable );
                }
            }

            return true;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: { // 17
            //qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE_UPDATE (17)(handled)";
            // Let the core know we have some variable changes if we set our internal flag (clear it so the change only happens once)
            // TODO: Protect all variable-touching code with mutexes?

            const bool updated = emulator->coreVarsUpdated();

            *static_cast<bool *>( data ) = updated;

            if ( updated ) {
                qDebug() << "cores updated";

                emulator->coreVarsUpdated( false );
            }

            return updated;
        }

        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: { // 18
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME (18) (handled)";

            if( !( *( const bool * )data ) ) {
                qCWarning( phxCore ) << "Core does not expect a game!";
            }

            return true;
        }

        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH: { // 19
            // This is done with the assumption that the core file path from setSource() will always be an absolute path
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LIBRETRO_PATH (19) (handled)";
            //*( const char ** )data = libretroCore.corePathCString;
            return true;
        }

            // 20 has been deprecated

        case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: { // 21
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK (21) (handled)";
            emulator->m_libretroLibrary.retro_frame_time = ( decltype( LibretroLibrary::retro_frame_time ) )data;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: // 22
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_AUDIO_CALLBACK (22)";
            break;

        case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: { // 23
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE (23) (handled)";
            static_cast<struct retro_rumble_interface *>( data )->set_rumble_state = &rumbleCallback;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES: { // 24
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES (24) (handled)";
            int64_t *caps = static_cast<int64_t *>( data );
            *caps = ( 1 << RETRO_DEVICE_JOYPAD ) | ( 1 << RETRO_DEVICE_ANALOG ) | ( 1 << RETRO_DEVICE_POINTER );
            return true;
        }

        case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE: // 25
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SENSOR_INTERFACE (RETRO_ENVIRONMENT_EXPERIMENTAL) (25)";
            break;

        case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: // 26
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CAMERA_INTERFACE (RETRO_ENVIRONMENT_EXPERIMENTAL) (26)";
            break;

        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: { // 27
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LOG_INTERFACE (27) (handled)";
            struct retro_log_callback *logcb = ( struct retro_log_callback * )data;
            logcb->log = logCallback;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: { // 28
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_PERF_INTERFACE (28) (handled)";
//            performanceCallback.get_cpu_features = 0;
//            libretroCore.performanceCallback.get_perf_counter = 0;
//            libretroCore.performanceCallback.get_time_usec = 0;
//            libretroCore.performanceCallback.perf_log = 0;
//            libretroCore.performanceCallback.perf_register = 0;
//            libretroCore.performanceCallback.perf_start = 0;
//            libretroCore.performanceCallback.perf_stop = 0;
//            *( retro_perf_callback * )data = libretroCore.performanceCallback;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE: // 29
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LOCATION_INTERFACE (29)";
            break;

        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY: { // 30
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY (30) (handled)";
            //*( const char ** )data = libretroCore.systemPathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: { // 31
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_SAVE_DIRECTORY (31) (handled)";
            //*( const char ** )data = libretroCore.savePathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: { // 32
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_systemAVInfo (32) (handled)";
            return true;
        }

        case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK: // 33
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK (33)";
            break;

        case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: // 34
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO (34)";
            break;

        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: { // 35
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_CONTROLLER_INFO (35) (handled)";

            // TODO: Return true once we handle this properly
            return false;

            for( const struct retro_controller_info *controllerInfo = ( const struct retro_controller_info * )data;
                 controllerInfo->types;
                 controllerInfo += sizeof( struct retro_controller_info )
                    ) {
                qDebug() << "Special controller:" << controllerInfo->types->desc;
            }
        }

        case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: //36
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_MEMORY_MAPS (RETRO_ENVIRONMENT_EXPERIMENTAL)(36)";
            break;

        case RETRO_ENVIRONMENT_SET_GEOMETRY: { // 37
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_GEOMETRY (37) (handled)";

            // Get info from the core
            emulator->m_libretroLibrary.retro_get_system_av_info( &emulator->m_avInfo );
            // Although we hope the core would have updated its internal av_info struct by now, we'll play it safe and
            // use the given geometry
            //memcpy( &( avInfo->geometry ), data, sizeof( struct retro_game_geometry ) );
            //libretroCore.getAVInfo( avInfo );

            return true;
        }

        case RETRO_ENVIRONMENT_GET_USERNAME: // 38
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_USERNAME (38)";
            break;

        case RETRO_ENVIRONMENT_GET_LANGUAGE: // 39
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_LANGUAGE (39)";
            break;

        case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER: // 40
            //qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER (RETRO_ENVIRONMENT_EXPERIMENTAL) (40)";
            break;

        default:
            qCDebug( phxCore ) << "Error: Environment command " << cmd << " is not defined in the frontend's libretro.h!.";
            return false;

    }

    // Command was not handled
    return false;
}

void RetroCallbacks::inputPollCallback() {

    emulator->m_gamepadManager.pollGamepads();
    //emulator->m_gamepadManager.pollKeys( emulator->m_sharedMemory );

}

int16_t RetroCallbacks::inputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id) {

    quint16 result = 0;

    // If there are no gamepads connected, just use the keyboard.
    if ( emulator->m_gamepadManager.isEmpty() ) {
        if ( device == RETRO_DEVICE_JOYPAD ) {
            const quint8 keyPress = emulator->m_gamepadManager.keyAt( id );
            result |= static_cast<qint16>( keyPress );
        }
    } else if ( emulator->m_gamepadManager.size() > port ) {
        const Gamepad &gamepad = emulator->m_gamepadManager.at( port );

        if ( device == RETRO_DEVICE_JOYPAD ) {
            result = static_cast<quint16>( gamepad.getButtonState( id ) );

            // OR the keyboard's state with the first controller's.
            if ( port == 0 ) {
                const quint8 keyPress = emulator->m_gamepadManager.keyAt( id );
                result |= static_cast<qint16>( keyPress );
            }
        }

        if ( device & RETRO_DEVICE_ANALOG ) {
            //qDebug() << "get analog";
        }
    }


    return result;
}

void RetroCallbacks::videoRefreshCallback(const void *t_data, unsigned width, unsigned height, size_t pitch) {

    if ( t_data == RETRO_HW_FRAME_BUFFER_VALID ) {

    } else {
        emulator->routeVideoFrame( static_cast<const char *>( t_data ), width
                , height
                , pitch );
    }

}

uintptr_t RetroCallbacks::getFramebufferCallback() {
    // Unsupported.
    return 0;
}

retro_proc_address_t RetroCallbacks::openGLProcAddressCallback( const char *sym ) {
    // Unsupported.
    return nullptr;
}

bool RetroCallbacks::rumbleCallback(unsigned port, retro_rumble_effect effect, uint16_t strength) {
    return false;
}

void RetroCallbacks::logCallback(retro_log_level level, const char *fmt, ...) {
    QVarLengthArray<char, 1024> outbuf( 1024 );
    va_list args;
    va_start( args, fmt );
    int ret = vsnprintf( outbuf.data(), outbuf.size(), fmt, args );

    if( ret < 0 ) {
        qCDebug( phxCore ) << "logCallback: could not format string";
        va_end( args );
        return;
    } else if( ( ret + 1 ) > outbuf.size() ) {
        outbuf.resize( ret + 1 );
        ret = vsnprintf( outbuf.data(), outbuf.size(), fmt, args );

        if( ret < 0 ) {
            qCDebug( phxCore ) << "logCallback: could not format string";
            va_end( args );
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
