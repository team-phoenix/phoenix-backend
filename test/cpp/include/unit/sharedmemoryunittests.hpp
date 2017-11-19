#pragma once

#include "catch.hpp"

#include <QtGlobal>

#include "sharedmemorybuffer.h"

TEST_CASE( "data is written at correct offset", "[write]" )
{

    GIVEN( "The mem buffer is created" ) {
        SharedMemoryBuffer buffer;
        uchar videoBuffer[ 5 ] = {
            0x00,
            0x01,
            0xAC,
            0x00,
            0x31,
        };
        for ( int i = 0; i < 5; ++i ) {
            uchar *output = buffer.writeVideoFrame( videoBuffer, 5 );
            REQUIRE( output[i] == videoBuffer[i] );
        }
    }
}
