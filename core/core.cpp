#include "core.h"

Core::Core( Node *parent ) : Node( parent ) {
}

// Slots

void Core::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    switch( command ) {
        case Command::Play: {
            state = State::Playing;
            break;
        }

        case Command::Stop: {
            state = State::Stopped;
            break;
        }

        case Command::Load: {
            state = State::Loading;
            emit commandOut( Command::Pause, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            state = State::Paused;
            break;
        }

        case Command::Pause: {
            state = State::Paused;
            break;
        }

        case Command::Unload: {
            state = State::Unloading;
            emit commandOut( Command::Stop, QVariant(), QDateTime::currentMSecsSinceEpoch() );
            state = State::Stopped;
            break;
        }

        case Command::SetPlaybackSpeed: {
            playbackSpeed = data.toReal();
            break;
        }

        case Command::SetVolume: {
            volume = data.toReal();
            break;
        }

        case Command::SetSource: {
            QMap<QString, QVariant> map = data.toMap();
            QStringMap stringMap;

            for( QString key : map.keys() ) {
                stringMap[ key ] = map[ key ].toString();
            }

            this->source = stringMap;
            break;
        }

        default: {
            break;
        }
    }
}
