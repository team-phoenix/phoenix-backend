#pragma once

#include <QIODevice>

class AudioBuffer : public QIODevice {
    Q_OBJECT

    public:
        AudioBuffer( QObject *parent = nullptr );
        ~AudioBuffer() = default;

        void start();
        void stop();
        void clear();

        qint64 readData( char *data, qint64 bytesToRead );
        qint64 writeData( const char *data, qint64 len );
        qint64 bytesAvailable() const;

    private:
        qint64 bufferPosition{ 0 };
        QByteArray buffer;

};
