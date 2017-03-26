#include "ringbuffer.h"

#include <QMutexLocker>

RingBuffer::RingBuffer( int _size )
{
    resize( _size );
}

void RingBuffer::resize(quint64 t_size) {
    m_buffer.resize( t_size );
    m_buffer.fill( '\0' );
    m_head = 0;
    m_tail = 0;
}

int RingBuffer::write(const char *_data, int _size) {

    int read = 0;

    for ( int i=0; i < _size; ++i ) {

        if ( m_tail == m_buffer.size() - 1 ) {
            m_tail = 0;
        }

        m_buffer[ m_tail ] = _data[ i ];

        m_tail++;

        if ( m_tail == m_head ) {
            qDebug() << "underrun";
            clear();
            read = 0;
            break;
        }


        read++;
    }

    return read;

}

int RingBuffer::readAvailable(char *_data) {

    int wrote = 0;

    const int size = m_tail - m_head;

    for ( int i=0; i < size; ++i ) {


        if ( m_head == m_buffer.size() - 1 ) {
            m_head = 0;
        }

        if ( m_head == m_tail ) {
            qDebug() << "underrun";
            clear();
            wrote = 0;
            break;
        }

        _data[ i ] = m_buffer[ m_head ];
        m_buffer[ m_head ] = '\0';

        m_head++;
        wrote++;

    }

    return wrote;
}

int RingBuffer::bytesAvailable() const {
    return m_tail.load() - m_head.load();
}

int RingBuffer::size() const {
    return m_buffer.size();
}

void RingBuffer::clear() {
    m_buffer.fill( '\0' );
    m_tail = 0;
    m_head = 0;
}
