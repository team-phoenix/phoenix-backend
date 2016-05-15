#include "libretrorunner.h"
#include "mousestate.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QString>
#include <QStringBuilder>
#include <QThread>
#include <QMutexLocker>

#include "SDL.h"
#include "SDL_gamecontroller.h"

void LibretroRunner::commandIn( Command command, QVariant data, qint64 timeStamp ) {
    // Command is not relayed to children automatically

    switch( command ) {
        case Command::Play: {
            // Make sure we're only connecting LibretroCore to this node if it's a command that only shows up when
            // emulation is active (in other words, never during loading)
            if( !connectedToCore ) {
                connectedToCore = true;
                qDebug() << "connection";
                connect( &core, &LibretroCore::dataOut, this, &LibretroRunner::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &LibretroRunner::commandOut );
            }

            qCDebug( phxCore ) << command;
            core.state = State::Playing;
            emit commandOut( Command::Play, QVariant(), nodeCurrentTime() );
            break;
        }

        case Command::Pause: {
            if( !connectedToCore ) {
                connectedToCore = true;
                qDebug() << "connection";
                connect( &core, &LibretroCore::dataOut, this, &LibretroRunner::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &LibretroRunner::commandOut );
            }

            qCDebug( phxCore ) << command;
            core.state = State::Paused;
            emit commandOut( Command::Pause, QVariant(), nodeCurrentTime() );
            break;
        }

        case Command::Stop: {
            if( !connectedToCore ) {
                connectedToCore = true;
                qDebug() << "connection";
                connect( &core, &LibretroCore::dataOut, this, &LibretroRunner::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &LibretroRunner::commandOut );
            }

            qCDebug( phxCore ) << command;
            core.state = State::Unloading;
            emit commandOut( Command::Unload, QVariant(), nodeCurrentTime() );

            // Write SRAM

            qCInfo( phxCore ) << "=======Saving game...=======";
            storeSaveData();
            qCInfo( phxCore ) << "============================";

            // Unload core

            // symbols.retro_api_version is reasonably expected to be defined if the core is loaded
            if( core.symbols.retro_api_version ) {
                core.symbols.retro_unload_game();
                core.symbols.retro_deinit();
                core.symbols.clear();
                core.coreFile.unload();
                qCDebug( phxCore ) << "Unloaded core successfully";
            } else {
                qCCritical( phxCore ) << "stop() called on an unloaded core!";
            }

            // Disconnect LibretroCore from the rest of the pipeline
            disconnect( &core, &LibretroCore::dataOut, this, &LibretroRunner::dataOut );
            disconnect( &core, &LibretroCore::commandOut, this, &LibretroRunner::commandOut );
            connectedToCore = false;

            if( core.fbo ) {
                delete core.fbo;
            }

            core.state = State::Stopped;
            emit commandOut( Command::Stop, QVariant(), nodeCurrentTime() );

            break;
        }

        // Run the emulator for a frame if we're supposed to
        case Command::Heartbeat: {
            // Drop any heartbeats from too far in the past
            if( nodeCurrentTime() - timeStamp > 50 ) {
                return;
            }

            emit commandOut( command, data, timeStamp );

            if( core.state == State::Playing ) {
                // If in 3D mode, lock the mutex before emulating
                if( core.videoFormat.videoMode == HARDWARERENDER ) {
                    core.videoMutex.lock();
                    //qDebug() << "LibretroRunner lock";
                    core.context->makeCurrent( core.surface );
                    core.fbo->bind();
                }

                // Invoke libretro core
                core.symbols.retro_run();

                if( core.videoFormat.videoMode == HARDWARERENDER ) {
                    core.context->makeCurrent( core.surface );
                    core.context->functions()->glFlush();
                    core.context->doneCurrent();
                    //qDebug() << "LibretroRunner unlock";
                    core.videoMutex.unlock();
                }
            }

            break;
        }

        case Command::SetWindowGeometry: {
            emit commandOut( command, data, timeStamp );
            core.windowGeometry = data.toRect();
            break;
        }

        case Command::SetAspectRatioMode: {
            core.aspectMode = data.toInt();
            break;
        }

        case Command::SetLibretroVariable: {
            LibretroVariable var = data.value<LibretroVariable>();
            core.variables.insert( var.key(), var );
            core.variablesAreDirty = true;
            break;
        }

        case Command::ControllerRemoved: {
            if( !connectedToCore ) {
                connectedToCore = true;
                qDebug() << "connection";
                connect( &core, &LibretroCore::dataOut, this, &LibretroRunner::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &LibretroRunner::commandOut );
            }

            GamepadState gamepad = data.value<GamepadState>();
            int instanceID = gamepad.instanceID;
            core.gamepads.remove( instanceID );
            emit commandOut( command, data, timeStamp );
            break;
        }

        default: {
            emit commandOut( command, data, timeStamp );
            break;
        }
    }
}

void LibretroRunner::dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    emit dataOut( type, mutex, data, bytes, timeStamp );

    switch( type ) {
        // Make a copy of the data into our own gamepad list
        case DataType::Input: {
            mutex->lock();
            GamepadState gamepad = *static_cast<GamepadState *>( data );
            mutex->unlock();
            int instanceID = gamepad.instanceID;
            core.gamepads[ instanceID ] = gamepad;
            break;
        }

        // Make a copy of the incoming data and store it
        case DataType::MouseInput: {
            mutex->lock();
            core.mouse = *static_cast<MouseState *>( data );
            mutex->unlock();
            break;
        }

        default:
            break;
    }
}
