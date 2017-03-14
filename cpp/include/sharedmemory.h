#pragma once

#include "QSharedMemory"

class QMutex;

class SharedMemory : public QSharedMemory {
    Q_OBJECT
public:

    ~SharedMemory();

    void setVideoMemory( uint t_width, uint t_height, uint t_pitch, const void *t_data );

    //void decodeInput( uint t_port, uint t_device, uint t_index, uint t_id );

    static SharedMemory &instance();

private:
    size_t m_videoBlockOffset;
    size_t m_inputBlockOffset;

    mutable QMutex *m_mutex;

    explicit SharedMemory( QObject *parent = nullptr );

    bool resizeMem( int t_blockSize );

};
