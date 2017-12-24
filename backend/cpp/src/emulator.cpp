#include "emulator.h"
#include "logging.h"
#include "retrocallbacks.h"

#include <QJsonArray>

#include <QCoreApplication>

Emulator::~Emulator() {
    shutdown();
}

void Emulator::setEmuState(Emulator::State t_state) {
    if ( t_state != m_emuState ) {
        m_emuState = t_state;
        sendState();
    }
}

void Emulator::setCallbacks() {
    // Set callbacks
    m_libretroLibrary.retro_set_environment( &RetroCallbacks::environmentCallback );
    m_libretroLibrary.retro_set_audio_sample( RetroCallbacks::audioSampleCallback );
    m_libretroLibrary.retro_set_audio_sample_batch( RetroCallbacks::audioSampleBatchCallback );
    m_libretroLibrary.retro_set_input_poll( RetroCallbacks::inputPollCallback );
    m_libretroLibrary.retro_set_input_state( RetroCallbacks::inputStateCallback );
    m_libretroLibrary.retro_set_video_refresh( RetroCallbacks::videoRefreshCallback );
}

bool Emulator::loadEmulationCore(const QString &t_emuCore) {

    qDebug() << t_emuCore;
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

void Emulator::handleVariableUpdate(const QByteArray &t_key, const QByteArray &t_value) {

    retro_variable variable;
    variable.key = t_key.data();
    variable.value = t_value.data();

    m_variableModel.insert( variable );
    coreVarsUpdated( true );

}

void Emulator::run() {

    m_libretroLibrary.retro_run();

    if ( m_emuState != State::Playing ) {
        setEmuState( State::Playing );
    }

}

void Emulator::init(const QString &t_corePath, const QString &t_gamePath, const QString &hwType) {

    if ( hwType == "2d" ) {

        if ( !( loadEmulationCore( t_corePath ) && loadEmulationGame( t_gamePath ) ) ) {
            qCWarning( phxCore, "Could not load the core or game." );
            return;
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

        emit m_audioController.audioFmtChanged( m_avInfo.timing.fps, m_avInfo.timing.sample_rate );

        qCDebug( phxCore ).nospace() << "coreFPS: " << m_avInfo.timing.fps;
        qCDebug( phxCore ).nospace() << "aspectRatio: " << m_avInfo.geometry.aspect_ratio;
        qCDebug( phxCore ).nospace() << "baseHeight: " << m_avInfo.geometry.base_height;
        qCDebug( phxCore ).nospace() << "baseWidth: " << m_avInfo.geometry.base_width;


        emit initialized();

        setEmuState( State::Initialized );
        sendVideoInfo();
        sendVariables();

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

void Emulator::shutdown() {

    // Calls retro_deinit().

    if ( m_emuState != State::Uninitialized && m_emuState != State::Killed ) {
        m_libretroLibrary.retro_deinit();
    }

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

void Emulator::restart() {
    shutdown();
}

Emulator::Emulator(QObject *parent) : AbstractEmulator( parent )
{

    m_systemInfo = {};
    m_avInfo = {};

    coreVarsUpdated( false );

    m_timer.setTimerType( Qt::PreciseTimer );
    m_timer.setInterval( 16 );

    connect( &m_timer, &QTimer::timeout, this, &Emulator::run );

    connect( &m_messageServer, &MessageServer::playEmu, &m_timer, static_cast<void(QTimer::*) (void)>( &QTimer::start ) );
    connect( &m_messageServer, &MessageServer::playEmu, &m_audioController, &AudioController::playEmu );

    connect( &m_messageServer, &MessageServer::pauseEmu, &m_timer, &QTimer::stop );
    connect( &m_messageServer, &MessageServer::shutdownEmu, this, &Emulator::shutdown );
    connect( &m_messageServer, &MessageServer::restartEmu, this, &Emulator::restart );

    connect( &m_messageServer, &MessageServer::initEmu, this, &Emulator::init );
    connect( &m_messageServer, &MessageServer::killEmu, this, &Emulator::kill );

    connect( &m_messageServer, &MessageServer::updateVariable, this, &Emulator::handleVariableUpdate );

    setEmuState( State::Uninitialized );
}

void Emulator::sendVariables() {

    for ( const CoreVariable &variable : m_variableModel ) {
        m_messageServer.encodeMessage( variable );
    }
}

void Emulator::kill() {
    setEmuState( State::Killed );
    QCoreApplication::quit();
}

void Emulator::sendVideoInfo() {

    const QJsonObject videoInfo {
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
