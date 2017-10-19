#pragma once

#include "QSharedMemory"

class QMutex;
class Gamepad;

class SharedMemory {
public:
     SharedMemory();
    ~SharedMemory();

     void readKeyboardStates( quint8 *m_keyboardStates, int t_size );
     void writeVideoFrame( uint t_width, uint t_height, uint t_pitch, const void *t_data );

private:
     QSharedMemory m_memory;

    size_t m_inputBlockSize;
    size_t m_videoBlockSize;

    size_t m_inputBlockOffset;
    size_t m_videoBlockOffset;


     bool resizeMem();

};
