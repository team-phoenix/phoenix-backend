#pragma once

#include "testing.h"

#include "QSharedMemory"

class QMutex;
class Gamepad;

class SharedMemory {
    friend class Test_SharedMemory;
    friend class Test_GamepadManager;
public:
     SharedMemory();
    ~SharedMemory();

    MOCKABLE void readKeyboardStates( quint8 *m_keyboardStates, int t_size );
    MOCKABLE void writeVideoFrame( uint t_width, uint t_height, uint t_pitch, const void *t_data );

private:

    size_t m_inputBlockSize;
    size_t m_videoBlockSize;

    size_t m_inputBlockOffset;
    size_t m_videoBlockOffset;

    QSharedMemory m_memory;

    MOCKABLE bool resizeMem();

};
