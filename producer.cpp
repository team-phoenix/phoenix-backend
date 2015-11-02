#include "producer.h"

ProducerFormat::ProducerFormat():
    audioFormat(),
    audioRatio( 1.0 ),
    videoBytesPerLine( 0 ),
    videoMode( SOFTWARERENDER ),
    videoFramerate( 60.0 ),
    videoPixelFormat( QImage::Format_RGB16 ),
    videoSize( 640, 480 ) {
}

ProducerFormat::~ProducerFormat() {

}

Producer::Producer() {
}

Producer::~Producer() {
}

