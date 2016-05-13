#include "libretrorunner.h"
#include "mousestate.h"

#include <QString>
#include <QStringBuilder>
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
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
            }

            qCDebug( phxCore ) << command;
            core.state = State::Playing;
            emit commandOut( Command::Play, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            break;
        }

        case Command::Pause: {
            if( !connectedToCore ) {
                connectedToCore = true;
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
            }

            qCDebug( phxCore ) << command;
            core.state = State::Paused;
            emit commandOut( Command::Pause, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            break;
        }

        case Command::Stop: {
            if( !connectedToCore ) {
                connectedToCore = true;
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
            }

            qCDebug( phxCore ) << command;
            core.state = State::Unloading;
            emit commandOut( Command::Unload, QVariant(), QDateTime::currentMSecsSinceEpoch() );

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
            disconnect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
            disconnect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
            connectedToCore = false;

            core.state = State::Stopped;
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

            if( core.state == State::Playing ) {
                // Invoke libretro core
                core.symbols.retro_run();
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
            core.updateVariables();
            break;
        }

        case Command::ControllerRemoved: {
            if( !connectedToCore ) {
                connectedToCore = true;
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
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
            if( !connectedToCore ) {
                connectedToCore = true;
                connect( &core, &LibretroCore::dataOut, this, &Node::dataOut );
                connect( &core, &LibretroCore::commandOut, this, &Node::commandOut );
            }

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
