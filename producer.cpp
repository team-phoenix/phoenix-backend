#include "producer.h"

ProducerFormat::ProducerFormat():
    producerType(),
    active( true ),
    firstFrame( true ),
    audioFormat(),
    audioRatio( 1.0 ),
    videoBytesPerLine( 0 ),
    videoFramerate( 60.0 ),
    videoMode( SOFTWARERENDER ),
    videoPixelFormat( QImage::Format_RGB16 ),
    videoSize( 640, 480 ) {
}

ProducerFormat::~ProducerFormat() {
}

Producer::Producer():
    producerFmt(),
    producerMutex() {
}

Producer::~Producer() {
}
