#include "videooutputnode.h"

#include "logging.h"

VideoOutputNode::VideoOutputNode( Node *parent ) : Node( parent ) {
}

void VideoOutputNode::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    emit commandOut( command, data, timeStamp );

    if( videoOutput ) {
        if( command == Command::VideoFormat ) {
            videoOutput->setFormat( qvariant_cast<ProducerFormat>( data ) );
        }

        switch( command ) {
            case Command::Stop:
                videoOutput->setState( State::Stopped );
                break;

            case Command::Load:
                videoOutput->setState( State::Loading );
                break;

            case Command::Play:
                videoOutput->setState( State::Playing );
                break;

            case Command::Pause:
                videoOutput->setState( State::Paused );
                break;

            case Command::Unload:
                videoOutput->setState( State::Unloading );
                break;

            case Command::VideoFormat:
                videoOutput->setFormat( qvariant_cast<ProducerFormat>( data ) );
                break;

            default:
                break;
        }
    }
}

void VideoOutputNode::dataIn( Node::DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    emit dataOut( type, mutex, data, bytes, timeStamp );

    if( videoOutput ) {
        if( type == DataType::Video ) {
            videoOutput->data( data, bytes, timeStamp );
        }
    }
}
