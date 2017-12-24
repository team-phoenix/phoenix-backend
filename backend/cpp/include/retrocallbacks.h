#pragma once

#include <libretro.h>
#include "abstractemulator.h"

class RetroCallbacks {
public:

    static void setEmulator( AbstractEmulator *emu );

// Required
    static void audioSampleCallback( int16_t left, int16_t right );

    static size_t audioSampleBatchCallback( const int16_t *data, size_t frames );

    static bool environmentCallback( unsigned cmd, void *data );

    static void inputPollCallback( void );

    static void logCallback( enum retro_log_level level, const char *fmt, ... );

    static int16_t inputStateCallback( unsigned port, unsigned device, unsigned index, unsigned id );

    static void videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch );

// Optional
    static uintptr_t getFramebufferCallback( void );

    static retro_proc_address_t openGLProcAddressCallback( const char *sym );

    static bool rumbleCallback( unsigned port, enum retro_rumble_effect effect, uint16_t strength );

private:
    RetroCallbacks();

};


