#include "producer.h"

ProducerFormat::ProducerFormat():
    producerType(),
    audioFormat(),
    audioRatio( 1.0 ),
    videoAspectRatio( 1.0 ),
    videoBytesPerLine( 0 ),
    videoBytesPerPixel( 0 ),
    videoFramerate( 60.0 ),
    videoMode( SOFTWARERENDER ),
    videoPixelFormat( QImage::Format_RGB16 ),
    videoSize( 720, 480 ) {
}

ProducerFormat::~ProducerFormat() {
}

Producer::Producer():
    producerFmt(),
    producerMutex() {
}

Producer::~Producer() {
}
