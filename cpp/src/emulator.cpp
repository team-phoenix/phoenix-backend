#include "emulator.h"
#include "sharedmemory.h"
#include "logging.h"
#include "gamepad.h"

#include <cstdarg>

#include <QLibrary>
#include <QFile>

#include <QTimer>

#include <QJsonObject>

#include <QCoreApplication>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>


Emulator &Emulator::instance() {
    static Emulator demuxer;
    return demuxer;
}

Emulator::~Emulator() {
}

void Emulator::setEmuState(Emulator::State t_state) {
    if ( t_state != m_emuState ) {
        m_emuState = t_state;
        sendState();
    }
}


void Emulator::setCallbacks() {
    // Set callbacks
    m_libretroLibrary.retro_set_environment( environmentCallback );
    m_libretroLibrary.retro_set_audio_sample( audioSampleCallback );
    m_libretroLibrary.retro_set_audio_sample_batch( audioSampleBatchCallback );
    m_libretroLibrary.retro_set_input_poll( inputPollCallback );
    m_libretroLibrary.retro_set_input_state( inputStateCallback );
    m_libretroLibrary.retro_set_video_refresh( videoRefreshCallback );
}

bool Emulator::loadEmulationCore(const QString &t_emuCore) {

    if ( t_emuCore != m_libretroLibrary.fileName() ) {

        return m_libretroLibrary.load( t_emuCore );

    } else {
        return false;
    }

}

bool Emulator::loadEmulationGame(const QString &t_emuGame) {

    if ( t_emuGame != m_game.fileName() ) {
        if ( !QFile::exists( t_emuGame ) ) {
            qCCritical( phxCore ) << t_emuGame << "does not exist.";
            return false;
        }

        m_gameFileName = t_emuGame.toLocal8Bit();
        m_game.setFileName( t_emuGame );

        return m_game.open( QIODevice::ReadOnly );
    }

    return false;
}

QString Emulator::toString(Emulator::State t_state) {
    switch( t_state ) {
        case State::Playing:
            return QStringLiteral( "emuPlaying" );
        case State::Initialized:
            return QStringLiteral( "emuInit" );
        case State::Paused:
            return QStringLiteral( "emuPaused" );
        case State::Uninitialized:
            return QStringLiteral( "emuUninit" );
        case State::Killed:
            return QStringLiteral( "emuKilled" );
        default:
            Q_UNREACHABLE();
    }
}

void Emulator::runEmu() {

    m_libretroLibrary.retro_run();

    if ( m_emuState != State::Playing ) {
        setEmuState( State::Playing );
    }

}

void Emulator::initEmu( const QString &t_corePath, const QString &t_gamePath, const QString &hwType ) {

    if ( hwType == "2d" ) {

        if ( !( loadEmulationCore( t_corePath ) && loadEmulationGame( t_gamePath ) ) ) {
            qCWarning( phxCore, "Could not load the core or game." );
        } else {

        }

        qCDebug( phxCore ) << "Core loaded:" << m_libretroLibrary.isLoaded();
        qCDebug( phxCore ) << "retro_api_version =" << m_libretroLibrary.retro_api_version();

        setCallbacks();

        // Init the core
        m_libretroLibrary.retro_init();

        // Get some info about the game
        m_libretroLibrary.retro_get_system_info( &m_systemInfo );

        // Load game
        {
            qCDebug( phxCore ) << "Loading game:" << m_game.fileName();

            // Argument struct for symbols.retro_load_game()
            retro_game_info gameInfo;

            // Full path needed, simply pass the game's file path to the core
            if( m_systemInfo.need_fullpath ) {
                qCDebug( phxCore ) << "Passing file path to core...";

                gameInfo.path = m_gameFileName.constData();
                gameInfo.data = nullptr;
                gameInfo.size = 0;
                gameInfo.meta = "";
            }

            // Full path not needed, read the file to memory and pass that to the core
            else {
                qCDebug( phxCore ) << "Copying game contents to memory...";

                // Read into memory

                gameInfo.path = nullptr;

                m_gameData = m_game.readAll();
                gameInfo.data = m_gameData.constData();
                gameInfo.size = m_gameData.size();
                gameInfo.meta = "";

                m_game.close();

            }

            m_libretroLibrary.retro_load_game( &gameInfo );

            qDebug() << "";
        }

        // Flush stderr, some cores may still write to it despite having RETRO_LOG
        fflush( stderr );

        // Load save data
        //LibretroCoreLoadSaveData();

        m_libretroLibrary.retro_get_system_av_info( &m_avInfo );
        qCDebug( phxCore ).nospace() << "coreFPS: " << m_avInfo.timing.fps;
        qCDebug( phxCore ).nospace() << "aspectRatio: " << m_avInfo.geometry.aspect_ratio;
        qCDebug( phxCore ).nospace() << "baseHeight: " << m_avInfo.geometry.base_height;
        qCDebug( phxCore ).nospace() << "baseWidth: " << m_avInfo.geometry.base_width;

        setEmuState( State::Initialized );
        sendVideoInfo();

    } else {
        qCWarning( phxCore, "hardware type %s is not supported", qPrintable( hwType ) );
    }


//        // Get audio/video timing and send to consumers, allocate buffer pool
//        {
//            // Get info from the core
//            retro_system_av_info *avInfo = new retro_system_av_info();
//            libretroCore.symbols.retro_get_system_av_info( avInfo );
//            LibretroCoreGrowBufferPool( avInfo );
//            qCDebug( phxCore ).nospace() << "coreFPS: " << avInfo->timing.fps;
//            emit commandOut( Command::SetCoreFPS, ( qreal )( avInfo->timing.fps ), nodeCurrentTime() );

//            // Create the FBO which contains the texture 3d cores will draw to
//            if( libretroCore.videoFormat.videoMode == HARDWARERENDER ) {
//                libretroCore.context->makeCurrent( libretroCore.surface );

//                // If the core has made available its max width/height at this stage, recreate the FBO with those settings
//                // Otherwise, use a sensible default size, the core will probably set the proper size in the first frame
//                if( avInfo->geometry.max_width != 0 && avInfo->geometry.max_height != 0 ) {
//                    if( libretroCore.fbo ) {
//                        delete libretroCore.fbo;
//                    }

//                    libretroCore.fbo = new QOpenGLFramebufferObject( avInfo->geometry.base_width, avInfo->geometry.base_height, QOpenGLFramebufferObject::CombinedDepthStencil );

//                    // Clear the newly created FBO
//                    libretroCore.fbo->bind();
//                    libretroCore.context->functions()->glClear( GL_COLOR_BUFFER_BIT );
//                } else {
//                    if( libretroCore.fbo ) {
//                        delete libretroCore.fbo;
//                    }

//                    libretroCore.fbo = new QOpenGLFramebufferObject( 640, 480, QOpenGLFramebufferObject::CombinedDepthStencil );

//                    // Clear the newly created FBO
//                    libretroCore.fbo->bind();
//                    libretroCore.context->functions()->glClear( GL_COLOR_BUFFER_BIT );

//                    avInfo->geometry.max_width = 640;
//                    avInfo->geometry.max_height = 480;
//                    avInfo->geometry.base_width = 640;
//                    avInfo->geometry.base_height = 480;
//                }

//                // Tell any video output children about this texture
//                emit commandOut( Command::SetOpenGLTexture, libretroCore.fbo->texture(), nodeCurrentTime() );

//                libretroCore.symbols.retro_hw_context_reset();
//            }

//            libretroCore.getAVInfo( avInfo );
//            delete avInfo;
//        }

    // Set all variables to their defaults, mark all variables as dirty
//        {
//            for( const auto &key : libretroCore.variables.keys() ) {
//                LibretroVariable &variable = libretroCore.variables[ key ];

//                if( !variable.choices().size() ) {
//                    continue;
//                }

//                // Assume the defualt choice to be the first option offered
//                QByteArray defaultChoice = variable.choices()[ 0 ];

//                if( defaultChoice.isEmpty() ) {
//                    continue;
//                }

//                // Assign
//                variable.setValue( defaultChoice );

//                QVariant var;
//                var.setValue( variable );
//                emit commandOut( Command::SetLibretroVariable, var, nodeCurrentTime() );
//            }

//            libretroCore.variablesAreDirty = true;
//        }

    // Flush stderr, some cores may still write to it despite having RETRO_LOG
    fflush( stderr );



}

void Emulator::shutdownEmu() {

    // Calls retro_deinit().
    m_libretroLibrary.unload();

    m_game.close();
    m_gameData.clear();

    m_gameFileName.clear();

    m_systemInfo = {};
    m_avInfo = {};

    m_pixelFormat = QImage::Format_Invalid;

    m_emuState = State::Uninitialized;

    setEmuState( State::Uninitialized );

}

void Emulator::restartEmu() {
    shutdownEmu();
}

Emulator::Emulator(QObject *parent) : QObject( parent ),
    m_pixelFormat( QImage::Format_Invalid )
{
    m_timer.setTimerType( Qt::PreciseTimer );
    m_timer.setInterval( 16 );

    connect( &m_timer, &QTimer::timeout, this, &Emulator::runEmu );

    connect( &m_messageServer, &MessageServer::playEmu, &m_timer, static_cast<void(QTimer::*) (void)>( &QTimer::start ) );
    connect( &m_messageServer, &MessageServer::pauseEmu, &m_timer, &QTimer::stop );
    connect( &m_messageServer, &MessageServer::shutdownEmu, this, &Emulator::shutdownEmu );
    connect( &m_messageServer, &MessageServer::restartEmu, this, &Emulator::restartEmu );

    connect( &m_messageServer, &MessageServer::initEmu, this, &Emulator::initEmu );
    connect( &m_messageServer, &MessageServer::killEmu, this, &Emulator::killEmu );

    setEmuState( State::Uninitialized );
}

void Emulator::killEmu() {
    setEmuState( State::Killed );
    QCoreApplication::quit();
}

void Emulator::sendVideoInfo() {

    QJsonObject videoInfo {
        { "response", "videoInfo" },
        { "frameRate", m_avInfo.timing.fps },
        { "aspectRatio", m_avInfo.geometry.aspect_ratio },
        { "height", static_cast<int>( m_avInfo.geometry.base_height ) },
        { "width", static_cast<int>( m_avInfo.geometry.base_width ) },
        { "pixelFmt", static_cast<int>( m_pixelFormat ) },
    };

    m_messageServer.encodeMessage( videoInfo );
}

void Emulator::sendState() {

    QJsonObject json {
        { "response", toString( m_emuState ) },
    };

    m_messageServer.encodeMessage( json );
}

void audioSampleCallback(int16_t left, int16_t right) {

}

size_t audioSampleBatchCallback(const int16_t *data, size_t frames) {

}

bool environmentCallback(unsigned cmd, void *data) {
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
                    Emulator::instance().m_pixelFormat = QImage::Format_RGB555;

//                    Emulator::instance().sendProcessMessage( {
//                                                                    { "retro_cmd", {
//                                                                          "environ_cb",
//                                                                      }
//                                                                }
//                                                                });
                    qCDebug( phxCore ) << "\t\tPixel format: 0RGB1555 aka QImage::Format_RGB555";
                    return true;

                case RETRO_PIXEL_FORMAT_RGB565:
                    Emulator::instance().m_pixelFormat  = QImage::Format_RGB16;
                    qCDebug( phxCore ) << "\t\tPixel format: RGB565 aka QImage::Format_RGB16";
                    return true;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    Emulator::instance().m_pixelFormat  = QImage::Format_RGB32;
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
            Emulator::instance().m_libretroLibrary.retro_keyboard_event = ( decltype( LibretroLibrary::retro_keyboard_event ) )data;
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

//            Emulator::instance().m_symbols.retro_hw_context_reset = hardwareRenderData->context_reset;

//            QSurfaceFormat surfaceFmt;

//            switch( hardwareRenderData->context_type ) {
//                case RETRO_HW_CONTEXT_NONE:
//                    qWarning() << "\t\tNo hardware context was selected";
//                    break;

//                case RETRO_HW_CONTEXT_OPENGL: {
//                    if( Emulator::instance().m_offscreenSurface->format().majorVersion() < 2
//                            || Emulator::instance().m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGL ) {
//                        qWarning() << "\t\tOpenGL 2.x not available! Please install a driver for your GPU that supports modern OpenGL";
//                        return false;
//                    }


//                    qDebug() << "\t\tOpenGL 2 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES2: {
//                    if( Emulator::instance().m_offscreenSurface->format().majorVersion() < 2
//                            || Emulator::instance().m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGLES ) {
//                        qWarning() << "\t\tOpenGL ES 2.0 not available! Please install a driver for your GPU that supports this";
//                        return false;
//                    }

//                    qDebug() << "\t\tOpenGL ES 2.0 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGL_CORE: {
//                    if( Emulator::instance().m_offscreenSurface->format().renderableType() == QSurfaceFormat::OpenGL ) {
//                        if( Emulator::instance().m_offscreenSurface->format().majorVersion() > static_cast<int>( hardwareRenderData->version_major ) ) {
//                            qDebug().nospace() << "\t\tOpenGL " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }

//                        if( Emulator::instance().m_offscreenSurface->format().majorVersion() >= static_cast<int>( hardwareRenderData->version_major ) &&
//                            Emulator::instance().m_offscreenSurface->format().minorVersion() >= static_cast<int>( hardwareRenderData->version_minor ) ) {
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
//                    if( Emulator::instance().m_offscreenSurface->format().majorVersion() < 3
//                            || Emulator::instance().m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGLES ) {
//                        qWarning() << "\t\tOpenGL ES 3.0 not available! Please install a driver for your GPU that supports this";
//                        return false;
//                    }

//                    qDebug() << "\t\tOpenGL ES 3.0 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES_VERSION: {
//                    if( Emulator::instance().m_offscreenSurface->format().renderableType() == QSurfaceFormat::OpenGLES ) {
//                        if( Emulator::instance().m_offscreenSurface->format().majorVersion() > static_cast<int>( hardwareRenderData->version_major ) ) {
//                            qDebug().nospace() << "\t\tOpenGL ES " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }

//                        if( Emulator::instance().m_offscreenSurface->format().majorVersion() >= static_cast<int>( hardwareRenderData->version_major ) &&
//                            Emulator::instance().m_offscreenSurface->format().minorVersion() >= static_cast<int>( hardwareRenderData->version_minor ) ) {
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
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE (15)(handled)";
//            auto *retroVariable = static_cast<struct retro_variable *>( data );

//            if( libretroCore.variables.contains( retroVariable->key ) ) {
//                const auto &var = libretroCore.variables[ retroVariable->key ];

//                if( var.isValid() ) {
//                    retroVariable->value = var.value().data();

//                    if( !var.value().isEmpty() ) {
//                        //qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE (15)(handled)" << var.key().c_str() << var.value().c_str();
//                        return true;
//                    }
//                }
//            }

            // Variable was not found
            return false;
        }

        case RETRO_ENVIRONMENT_SET_VARIABLES: { // 16
            qCDebug( phxCore ) << "RETRO_ENVIRONMENT_SET_VARIABLES (16) (handled)";
//            auto *rv = ( const struct retro_variable * )( data );

//            for( ; rv->key != NULL; rv++ ) {
//                LibretroVariable v( rv );

//                if( !( libretroCore.variables.contains( v.key() ) ) ) {
//                    libretroCore.variables.insert( v.key(), v );
//                }

//            }

            return true;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: { // 17
            // qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_GET_VARIABLE_UPDATE (17)(handled)";
            // Let the core know we have some variable changes if we set our internal flag (clear it so the change only happens once)
            // TODO: Protect all variable-touching code with mutexes?
//            if( libretroCore.variablesAreDirty ) {
//                // Special case: Force DeSmuME's pointer type variable to "touch" in order to work with our touch code
//                if( libretroCore.variables.contains( "desmume_pointer_type" ) ) {
//                    libretroCore.variables[ "desmume_pointer_type" ].setValue( QByteArrayLiteral( "touch" ) );
//                }

//                libretroCore.variablesAreDirty = false;
//                *static_cast<bool *>( data ) = true;
//                return true;
//            } else {
//                *static_cast<bool *>( data ) = false;
//                return false;
//            }


            break;
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
            Emulator::instance().m_libretroLibrary.retro_frame_time = ( decltype( LibretroLibrary::retro_frame_time ) )data;
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
            Emulator::instance().m_libretroLibrary.retro_get_system_av_info( &Emulator::instance().m_avInfo );
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

void inputPollCallback() {
    Emulator::instance().m_gamepadManager.poll();
}

int16_t inputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id) {

    quint16 result = 0;

    if ( Emulator::instance().m_gamepadManager.size() > port ) {
        const Gamepad &gamepad = Emulator::instance().m_gamepadManager.at( port );

        if ( device & RETRO_DEVICE_JOYPAD ) {
            result = static_cast<quint16>( gamepad.getButtonState( id ) );
        }

        if ( device & RETRO_DEVICE_ANALOG ) {
            //qDebug() << "get analog";
        }
    }


    return result;
}

void videoRefreshCallback(const void *t_data, unsigned width, unsigned height, size_t pitch) {

    if ( t_data == RETRO_HW_FRAME_BUFFER_VALID ) {

    } else {
        SharedMemory::instance().setVideoMemory( width
                                                        , height
                                                        , pitch
                                                        , t_data );
    }

}

uintptr_t getFramebufferCallback() {
    // Unsupported.
    return 0;
}

retro_proc_address_t openGLProcAddressCallback( const char *sym ) {
    // Unsupported.
    return nullptr;
}

bool rumbleCallback(unsigned port, retro_rumble_effect effect, uint16_t strength) {
    return false;
}

void logCallback(retro_log_level level, const char *fmt, ...) {
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
