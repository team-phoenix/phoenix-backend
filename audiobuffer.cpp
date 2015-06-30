#include "audiobuffer.h"

AudioBuffer::AudioBuffer( QObject *parent )
    : QIODevice( parent ),
      bufferPosition( 0 ) {

}

AudioBuffer::~AudioBuffer() {

}

void AudioBuffer::start() {

    open( QIODevice::ReadWrite );

}

void AudioBuffer::stop() {

    bufferPosition = 0;
    close();

}

qint64 AudioBuffer::readData( char *data, qint64 bytesToRead ) {

    if( buffer.size() > bytesToRead ) {

        memcpy( data, buffer.constData(), bytesToRead );
        buffer.remove( 0, bytesToRead );
        return bytesToRead;

    } else {

        memcpy( data, buffer.constData(), buffer.size() );
        int bytesRead = buffer.size();
        buffer.clear();
        return bytesRead;

    }

    return 0;

}

qint64 AudioBuffer::writeData( const char *data, qint64 len ) {

    buffer.append( data, len );

    return 0;
}

qint64 AudioBuffer::bytesAvailable() const {

    return buffer.size() + QIODevice::bytesAvailable();

}

