#pragma once

#include <QByteArray>
#include <QDebug>

#include <QMutex>
#include <atomic>

class RingBuffer
{
public:
    explicit RingBuffer( int _size = 0 );

    void resize( quint64 t_size );

    int write( const char* _data, int _size );

    int readAvailable( char *t_data );

    int bytesAvailable() const;

    int size() const;

    void clear();

    const char *data() const {
        return m_buffer.data();
    }

private:
    int m_head;
    int m_tail;

    QByteArray m_buffer;

};
