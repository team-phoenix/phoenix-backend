#include "coredemuxer.h"
#include "sharedprocessmemory.h"
#include "logging.h"
#include "inputmanager.h"

#include <QLibrary>
#include <QFile>

#include <QTimer>

#include <QLocalServer>
#include <QLocalSocket>

#include <QJsonDocument>
#include <QJsonObject>

#include <QCoreApplication>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>


CoreDemuxer &CoreDemuxer::instance() {
    static CoreDemuxer demuxer;
    return demuxer;
}

CoreDemuxer::~CoreDemuxer() {
}

void CoreDemuxer::loadLibrary(QString t_lib) {
    if ( !QFile::exists( t_lib ) ) {
        qCDebug( phxCore ) << t_lib << "does not exist.";
    }

    if ( m_coreLib->isLoaded() ) {
        resetSession();
    }
    m_coreLib->setFileName( t_lib );
    m_coreLib->load();
    Q_ASSERT( m_coreLib->isLoaded() );

    emit init();

}

void CoreDemuxer::loadGame(QString t_game) {
    if ( !QFile::exists( t_game ) ) {
        qCDebug( phxCore ) << t_game << "does not exist.";
    }

    if ( m_game->isOpen() ) {
        resetSession();
    }

    m_gameFileName = t_game.toLocal8Bit();
    m_game->setFileName( t_game );
    m_game->open( QIODevice::ReadOnly );
    Q_ASSERT( m_game->isOpen() );

    emit init();
}

void CoreDemuxer::sendProcessMessage(QString t_section, QString t_value) {
    sendProcessMessage( { { t_section, t_value } } );
}

void CoreDemuxer::sendProcessMessage(QJsonObject &&t_message) {
    sendProcessMessage( t_message );
}

void CoreDemuxer::sendProcessMessage( const QJsonObject &t_message ) {
    emit pipeMessage( QJsonDocument( t_message ).toBinaryData() );
}

void CoreDemuxer::run() {
    static bool ranOnce = false;

    m_symbols.retro_run();

    if ( !ranOnce ) {
        sendProcessMessage( "stateChanged", "playing" );
        ranOnce = true;
    }
}

void CoreDemuxer::handleInit() {
    if ( !m_game->isOpen() && m_coreLib->isLoaded() ) {
        return;
    }

    QJsonObject resultMessage;


    m_symbols.resolveSymbols( *m_coreLib );

    qCDebug( phxCore ) << "Core loaded:" << m_coreLib->isLoaded();
    qCDebug( phxCore ) << "retro_api_version =" << m_symbols.retro_api_version();

    // Set callbacks
    m_symbols.retro_set_environment( environmentCallback );
    m_symbols.retro_set_audio_sample( audioSampleCallback );
    m_symbols.retro_set_audio_sample_batch( audioSampleBatchCallback );
    m_symbols.retro_set_input_poll( inputPollCallback );
    m_symbols.retro_set_input_state( inputStateCallback );
    m_symbols.retro_set_video_refresh( videoRefreshCallback );

    // Init the core
    m_symbols.retro_init();

    qDebug() << "max height:" <<m_avInfo.geometry.base_width;
    // Get some info about the game
    m_symbols.retro_get_system_info( &m_systemInfo );

    // Load game
    {
        qCDebug( phxCore ) << "Loading game:" << m_game->fileName();

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

            m_gameData = m_game->readAll();
            gameInfo.data = m_gameData.constData();
            gameInfo.size = m_gameData.size();
            gameInfo.meta = "";

            m_game->close();

        }

        m_symbols.retro_load_game( &gameInfo );

        qDebug() << "";
    }

    // Flush stderr, some cores may still write to it despite having RETRO_LOG
    fflush( stderr );

    // Load save data
    //LibretroCoreLoadSaveData();

    m_symbols.retro_get_system_av_info( &m_avInfo );
    qCDebug( phxCore ).nospace() << "coreFPS: " << m_avInfo.timing.fps;
    qCDebug( phxCore ).nospace() << "aspectRatio: " << m_avInfo.geometry.aspect_ratio;
    qCDebug( phxCore ).nospace() << "baseHeight: " << m_avInfo.geometry.base_height;
    qCDebug( phxCore ).nospace() << "baseWidth: " << m_avInfo.geometry.base_width;


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

    sendVideoInfo();

}

void CoreDemuxer::handleSocketRead( QJsonObject &t_message ) {
   qDebug() << "Accepted pipe request:" << t_message << t_message.isEmpty();

   if ( !t_message.isEmpty() ) {
       for ( auto iter = t_message.begin(); iter != t_message.end(); ++iter ) {
           QString value = iter.value().toString();

           if ( iter.key() == "loadCore" ) {
               loadLibrary( value );
           } else if ( iter.key() == "loadGame" ) {
               loadGame( value );
           } else if ( iter.key() == "setState" ) {

               if ( iter.value() == "play" ) {
                   QTimer *timer = new QTimer( this );
                   timer->setInterval( ( 1 / m_avInfo.timing.fps ) * 1000.0 );

                   connect( timer, &QTimer::timeout, this, &CoreDemuxer::run );

                   qDebug() << "sent message" << "playing";
               }

           } else if ( iter.key() == "quitProcess" ) {
               qDebug() << "QUIT PROCESS";

               QCoreApplication::quit();
           }
       }
   }

}

CoreDemuxer::CoreDemuxer(QObject *parent)
    : QObject( parent ),

      m_pixelFormat( QImage::Format_Invalid ),

      m_coreLib( new QLibrary( this ) ),
      m_game( new QFile( this ) ),
      m_inputManager( new InputManager( nullptr ) )
{
    connect( this, &CoreDemuxer::init, this, &CoreDemuxer::handleInit );

    QLocalServer *server = new QLocalServer( this );
    const QString serverName = "phoenixEmulatorProcess";
    QLocalServer::removeServer(serverName);
    if ( !server->listen( serverName ) ) {
        qCDebug( phxCore ) << "QLocalSocket is not started...";
    } else {
        qDebug() << "Backend server is listening";
    }

    connect( server, &QLocalServer::newConnection, this, [this, server] {
        QLocalSocket *socket = server->nextPendingConnection();

        qDebug() << "Connected to a new process.";

        // Listen to incoming pipes from external processes, we can we
        // fulfill their request.
        connect( socket, &QLocalSocket::readyRead, this, [ this, socket ] {

            if ( socket->bytesAvailable() > 4 ) {
                while ( socket->bytesAvailable() ) {

                    quint32 msgSize = 0;
                    socket->read( reinterpret_cast<char *>( &msgSize ), sizeof( msgSize ) );


                    if ( socket->bytesAvailable() >= msgSize ) {
                        QByteArray socketMsg( msgSize, '\0' );

                        socket->read( socketMsg.data(), msgSize );

                        qDebug() << socketMsg;

                        QJsonObject obj = QJsonDocument::fromBinaryData( socketMsg ).object();
                        handleSocketRead( obj );
                    }
                }

            }

        });

        // Connect to a frontend server, so we can request information from it.
        QLocalSocket *outSocket = new QLocalSocket( this );
        outSocket->connectToServer( "phoenixExternalProcess" );

        connect( this, &CoreDemuxer::pipeMessage, this, [this, outSocket] ( QByteArray t_data ) {
            quint32 size = static_cast<quint32>( t_data.size() );
            outSocket->write( reinterpret_cast<char *>( &size ), sizeof( size ) );
            outSocket->write( t_data );
            outSocket->flush();
        });

        connect( outSocket, &QLocalSocket::connected, this, [this] {
           qCDebug( phxCore ) << "Connected to Frontend";
        });

    });


}

void CoreDemuxer::resetSession() {
    m_coreLib->unload();
    m_game->close();
}

void CoreDemuxer::shutdownSession() {
    m_coreLib->unload();
    m_game->close();

}

bool CoreDemuxer::sessionReady() {
    return m_coreLib->isLoaded() && m_game->isOpen();
}

void CoreDemuxer::sendVideoInfo() {

    QJsonObject videoInfo;

    videoInfo[ "videoInfo" ] = QJsonValue( {
        { "frameRate", m_avInfo.timing.fps },
        { "aspectRatio", m_avInfo.geometry.aspect_ratio },
        { "baseHeight", static_cast<int>( m_avInfo.geometry.base_height ) },
        { "baseWidth", static_cast<int>( m_avInfo.geometry.base_width ) },
        { "pixelFormat" , static_cast<int>( m_pixelFormat ) },
    } );

    sendProcessMessage( videoInfo  );
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
            //*( const char ** )data = libretroCore.systemPathCString;
            return true;
        }

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: { // 10
            qCDebug( phxCore ) << "\tRETRO_ENVIRONMENT_SET_PIXEL_FORMAT (10) (handled)";

            retro_pixel_format *pixelformat = static_cast<enum retro_pixel_format *> ( data );


            switch( *pixelformat ) {
                case RETRO_PIXEL_FORMAT_0RGB1555:
                    CoreDemuxer::instance().m_pixelFormat = QImage::Format_RGB555;

//                    CoreDemuxer::instance().sendProcessMessage( {
//                                                                    { "retro_cmd", {
//                                                                          "environ_cb",
//                                                                      }
//                                                                }
//                                                                });
                    qCDebug( phxCore ) << "\t\tPixel format: 0RGB1555 aka QImage::Format_RGB555";
                    return true;

                case RETRO_PIXEL_FORMAT_RGB565:
                    CoreDemuxer::instance().m_pixelFormat  = QImage::Format_RGB16;
                    qCDebug( phxCore ) << "\t\tPixel format: RGB565 aka QImage::Format_RGB16";
                    return true;

                case RETRO_PIXEL_FORMAT_XRGB8888:
                    CoreDemuxer::instance().m_pixelFormat  = QImage::Format_RGB32;
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
            CoreDemuxer::instance().m_symbols.retro_keyboard_event = ( decltype( CoreSymbols::retro_keyboard_event ) )data;
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

//            CoreDemuxer::instance().m_symbols.retro_hw_context_reset = hardwareRenderData->context_reset;

//            QSurfaceFormat surfaceFmt;

//            switch( hardwareRenderData->context_type ) {
//                case RETRO_HW_CONTEXT_NONE:
//                    qWarning() << "\t\tNo hardware context was selected";
//                    break;

//                case RETRO_HW_CONTEXT_OPENGL: {
//                    if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() < 2
//                            || CoreDemuxer::instance().m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGL ) {
//                        qWarning() << "\t\tOpenGL 2.x not available! Please install a driver for your GPU that supports modern OpenGL";
//                        return false;
//                    }


//                    qDebug() << "\t\tOpenGL 2 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES2: {
//                    if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() < 2
//                            || CoreDemuxer::instance().m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGLES ) {
//                        qWarning() << "\t\tOpenGL ES 2.0 not available! Please install a driver for your GPU that supports this";
//                        return false;
//                    }

//                    qDebug() << "\t\tOpenGL ES 2.0 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGL_CORE: {
//                    if( CoreDemuxer::instance().m_offscreenSurface->format().renderableType() == QSurfaceFormat::OpenGL ) {
//                        if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() > static_cast<int>( hardwareRenderData->version_major ) ) {
//                            qDebug().nospace() << "\t\tOpenGL " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }

//                        if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() >= static_cast<int>( hardwareRenderData->version_major ) &&
//                            CoreDemuxer::instance().m_offscreenSurface->format().minorVersion() >= static_cast<int>( hardwareRenderData->version_minor ) ) {
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
//                    if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() < 3
//                            || CoreDemuxer::instance().m_offscreenSurface->format().renderableType() != QSurfaceFormat::OpenGLES ) {
//                        qWarning() << "\t\tOpenGL ES 3.0 not available! Please install a driver for your GPU that supports this";
//                        return false;
//                    }

//                    qDebug() << "\t\tOpenGL ES 3.0 context was selected";
//                    return true;
//                }

//                case RETRO_HW_CONTEXT_OPENGLES_VERSION: {
//                    if( CoreDemuxer::instance().m_offscreenSurface->format().renderableType() == QSurfaceFormat::OpenGLES ) {
//                        if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() > static_cast<int>( hardwareRenderData->version_major ) ) {
//                            qDebug().nospace() << "\t\tOpenGL ES " << hardwareRenderData->version_major << "." << hardwareRenderData->version_minor << " "
//                                               << "context was selected";
//                            return true;
//                        }

//                        if( CoreDemuxer::instance().m_offscreenSurface->format().majorVersion() >= static_cast<int>( hardwareRenderData->version_major ) &&
//                            CoreDemuxer::instance().m_offscreenSurface->format().minorVersion() >= static_cast<int>( hardwareRenderData->version_minor ) ) {
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
            CoreDemuxer::instance().m_symbols.retro_frame_time = ( decltype( CoreSymbols::retro_frame_time ) )data;
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
            //logcb->log = logCallback;
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
            CoreDemuxer::instance().m_symbols.retro_get_system_av_info( &CoreDemuxer::instance().m_avInfo );
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

}

void logCallback( retro_log_level level, const char *fmt ) {
//    QVarLengthArray<char, 1024> outbuf( 1024 );
//    va_list args;
//    va_start( args, fmt );
//    int ret = vsnprintf( outbuf.data(), outbuf.size(), fmt, args );

//    if( ret < 0 ) {
//        qCDebug( phxCore ) << "logCallback: could not format string";
//        va_end( args );
//        return;
//    } else if( ( ret + 1 ) > outbuf.size() ) {
//        outbuf.resize( ret + 1 );
//        ret = vsnprintf( outbuf.data(), outbuf.size(), fmt, args );

//        if( ret < 0 ) {
//            qCDebug( phxCore ) << "logCallback: could not format string";
//            va_end( args );
//            return;
//        }
//    }

//    va_end( args );

//    // remove trailing newline, which are already added by qCDebug
//    if( outbuf.value( ret - 1 ) == '\n' ) {
//        outbuf[ret - 1] = '\0';

//        if( outbuf.value( ret - 2 ) == '\r' ) {
//            outbuf[ret - 2] = '\0';
//        }
//    }

//    switch( level ) {
//        case RETRO_LOG_DEBUG:
//            qCDebug( phxCore ) << "RETRO_LOG_DEBUG:" << outbuf.data();
//            break;

//        case RETRO_LOG_INFO:
//            qCDebug( phxCore ) << "RETRO_LOG_INFO:" << outbuf.data();
//            break;

//        case RETRO_LOG_WARN:
//            qCWarning( phxCore ) << "RETRO_LOG_WARN:" << outbuf.data();
//            break;

//        case RETRO_LOG_ERROR:
//            qCCritical( phxCore ) << "RETRO_LOG_ERROR:" << outbuf.data();
//            break;

//        default:
//            qCWarning( phxCore ) << "RETRO_LOG (unknown category!?):" << outbuf.data();
//            break;
//    }
}

int16_t inputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id) {
    return 0;
}

void videoRefreshCallback(const void *t_data, unsigned width, unsigned height, size_t pitch) {

    if ( t_data == RETRO_HW_FRAME_BUFFER_VALID ) {

    } else {
        SharedProcessMemory::instance().setVideoMemory( width
                                                        , height
                                                        , pitch
                                                        , t_data );
    }

}

uintptr_t getFramebufferCallback() {
    return 0;
}

retro_proc_address_t openGLProcAddressCallback( const char *sym ) {
    //return CoreDemuxer::instance().m_openglContext->getProcAddress( sym );

    return nullptr;
}

bool rumbleCallback(unsigned port, retro_rumble_effect effect, uint16_t strength) {
    return false;
}
