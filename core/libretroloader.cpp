#include "libretroloader.h"

#include <QOffscreenSurface>
#include <QString>
#include <QStringBuilder>

#include "SDL.h"
#include "SDL_gamecontroller.h"

void LibretroLoader::commandIn( Command command, QVariant data, qint64 timeStamp ) {
    // Command is not relayed to children automatically

    switch( command ) {
        case Command::Load: {
            // Make sure we only connect on load
            if( !connectedToCore ) {
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
                connectedToCore = true;
            }

            qCDebug( phxCore ) << command;
            core.state = State::Loading;
            emit commandOut( Command::Load, QVariant(), QDateTime::currentMSecsSinceEpoch() );

            // Set paths (QFileInfo gives you convenience functions, for example to extract just the directory from a file path)
            core.coreFileInfo.setFile( core.source[ "core" ] );
            core.gameFileInfo.setFile( core.source[ "game" ] );
            core.systemPathInfo.setFile( core.source[ "systemPath" ] );
            core.savePathInfo.setFile( core.source[ "savePath" ] );

            core.coreFile.setFileName( core.coreFileInfo.absoluteFilePath() );
            core.gameFile.setFileName( core.gameFileInfo.absoluteFilePath() );

            core.contentPath.setPath( core.gameFileInfo.absolutePath() );
            core.systemPath.setPath( core.systemPathInfo.absolutePath() );
            core.savePath.setPath( core.savePathInfo.absolutePath() );

            // Convert to C-style ASCII strings (needed by the API)
            core.corePathByteArray = core.coreFileInfo.absolutePath().toLocal8Bit();
            core.gameFileByteArray = core.gameFileInfo.absoluteFilePath().toLocal8Bit();
            core.gamePathByteArray = core.gameFileInfo.absolutePath().toLocal8Bit();
            core.systemPathByteArray = core.systemPathInfo.absolutePath().toLocal8Bit();
            core.savePathByteArray = core.savePathInfo.absolutePath().toLocal8Bit();
            core.corePathCString = core.corePathByteArray.constData();
            core.gameFileCString = core.gameFileByteArray.constData();
            core.gamePathCString = core.gamePathByteArray.constData();
            core.systemPathCString = core.systemPathByteArray.constData();
            core.savePathCString = core.savePathByteArray.constData();

            qDebug() << "";
            qCDebug( phxCore ) << "Now loading:";
            qCDebug( phxCore ) << "Core        :" << core.source[ "core" ];
            qCDebug( phxCore ) << "Game        :" << core.source[ "game" ];
            qCDebug( phxCore ) << "System path :" << core.source[ "systemPath" ];
            qCDebug( phxCore ) << "Save path   :" << core.source[ "savePath" ];
            qDebug() << "";

            // Set defaults that'll get overwritten as the core loads if necessary
            {
                // Pixel format is set to QImage::Format_RGB16 by default by the struct ProducerFormat constructor
                // However, for Libretro the default is RGB1555 aka QImage::Format_RGB555
                core.videoFormat.videoPixelFormat = QImage::Format_RGB555;
            }

            // Load core
            {
                qCDebug( phxCore ) << "Loading core:" << core.coreFileInfo.absoluteFilePath();

                core.coreFile.load();

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
                core.symbols.retro_set_environment( environmentCallback );
                core.symbols.retro_set_audio_sample( audioSampleCallback );
                core.symbols.retro_set_audio_sample_batch( audioSampleBatchCallback );
                core.symbols.retro_set_input_poll( inputPollCallback );
                core.symbols.retro_set_input_state( inputStateCallback );
                core.symbols.retro_set_video_refresh( videoRefreshCallback );

                // Init the core
                core.symbols.retro_init();

                // Get some info about the game
                core.symbols.retro_get_system_info( core.systemInfo );

                qDebug() << "";
            }

            // Load game
            {
                qCDebug( phxCore ) << "Loading game:" << core.gameFileInfo.absoluteFilePath();

                // Argument struct for symbols.retro_load_game()
                retro_game_info gameInfo;

                // Full path needed, simply pass the game's file path to the core
                if( core.systemInfo->need_fullpath ) {
                    qCDebug( phxCore ) << "Passing file path to core...";
                    gameInfo.path = core.gameFileCString;
                    gameInfo.data = nullptr;
                    gameInfo.size = 0;
                    gameInfo.meta = "";
                }

                // Full path not needed, read the file to memory and pass that to the core
                else {
                    qCDebug( phxCore ) << "Copying game contents to memory...";
                    core.gameFile.open( QIODevice::ReadOnly );

                    // read into memory
                    core.gameData = core.gameFile.readAll();

                    gameInfo.path = nullptr;
                    gameInfo.data = core.gameData.constData();
                    gameInfo.size = core.gameFile.size();
                    gameInfo.meta = "";
                }

                core.symbols.retro_load_game( &gameInfo );

                qDebug() << "";
            }

            // Load save data
            loadSaveData();

            // Get audio/video timing and send to consumers, allocate buffer pool
            {
                // Get info from the core
                retro_system_av_info *avInfo = new retro_system_av_info();
                core.symbols.retro_get_system_av_info( avInfo );
                allocateBufferPool( avInfo );
                qCDebug( phxCore ).nospace() << "coreFPS: " << avInfo->timing.fps;
                emit commandOut( Command::SetCoreFPS, ( qreal )( avInfo->timing.fps ), QDateTime::currentMSecsSinceEpoch() );

                // Create the FBO 3d cores will draw to
                if( core.videoFormat.videoMode == HARDWARERENDER ) {
                    core.context->makeCurrent( core.surface );

                    // If the core has made available its max width/height at this stage, recreate the FBO with those settings
                    // Otherwise, use the default values set in PhoenixWindow
                    if( ( avInfo->geometry.max_width != 0 && avInfo->geometry.max_height != 0 ) && core.fbo ) {
                        delete core.fbo;

                        core.fbo = new QOpenGLFramebufferObject( avInfo->geometry.base_width, avInfo->geometry.base_height, QOpenGLFramebufferObject::CombinedDepthStencil );
                    } else {
                        if( !core.fbo ) {
                            core.fbo = new QOpenGLFramebufferObject( 640, 480, QOpenGLFramebufferObject::CombinedDepthStencil );
                        }

                        avInfo->geometry.max_width = 640;
                        avInfo->geometry.max_height = 480;
                        avInfo->geometry.base_width = 640;
                        avInfo->geometry.base_height = 480;
                    }

                    // Tell any video output children about this texture
                    emit commandOut( Command::SetOpenGLTexture, core.fbo->texture(), nodeCurrentTime() );

                    core.symbols.retro_hw_context_reset();
                }

                core.getAVInfo( avInfo );
                delete avInfo;
            }

            // Set all variables to their defaults, mark all variables as dirty
            {
                for( const auto &key : core.variables.keys() ) {
                    LibretroVariable &variable = core.variables[ key ];

                    if( !variable.choices().size() ) {
                        continue;
                    }

                    // Assume the defualt choice to be the first option offered
                    QByteArray defaultChoice = variable.choices()[ 0 ];

                    if( defaultChoice.isEmpty() ) {
                        continue;
                    }

                    // Assign
                    variable.setValue( defaultChoice );

                    QVariant var;
                    var.setValue( variable );
                    emit commandOut( Command::SetLibretroVariable, var, nodeCurrentTime() );
                }

                core.variablesAreDirty = true;
            }

            // Disconnect LibretroCore from the rest of the pipeline
            disconnect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
            disconnect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
            connectedToCore = false;

            core.pausable = true;
            emit commandOut( Command::SetPausable, true, QDateTime::currentMSecsSinceEpoch() );

            core.state = State::Paused;
            emit commandOut( Command::Pause, QVariant(), QDateTime::currentMSecsSinceEpoch() );

            break;
        }

        case Command::SetSource: {
            // Make sure we only connect on load
            if( !connectedToCore ) {
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
                connectedToCore = true;
            }

            qCDebug( phxCore ) << command;
            emit commandOut( command, data, timeStamp );

            QMap<QString, QVariant> map = data.toMap();
            QStringMap stringMap;

            for( QString key : map.keys() ) {
                stringMap[ key ] = map[ key ].toString();
            }

            core.source = stringMap;
            break;
        }

        case Command::SetSurface: {
            emit commandOut( command, data, timeStamp );
            core.surface = data.value<QOffscreenSurface *>();
            break;
        }

        case Command::SetOpenGLContext: {
            emit commandOut( command, data, timeStamp );
            core.context = data.value<QOpenGLContext *>();
            break;
        }

        case Command::SetOpenGLFBO: {
            emit commandOut( command, data, timeStamp );
            core.fbo = static_cast<QOpenGLFramebufferObject *>( data.value<void *>() );
            break;
        }

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}
