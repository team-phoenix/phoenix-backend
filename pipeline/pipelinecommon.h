#pragma once

#include <QObject>
#include <QAudioFormat>
#include <QImage>

/*
 * Structures usable by all nodes
 */

// Type of video output (for use by video consumers)
enum VideoRendererType {

    // Video is generated by the CPU and lives in RAM
    SOFTWARERENDER = 0,

    // Video is generated by the GPU and lives in an FBO
    HARDWARERENDER

};

struct ProducerFormat {
    ProducerFormat() = default;
    ~ProducerFormat() = default;

    // Control

    // "libretro", etc.
    QString producerType;

    // Audio

    QAudioFormat audioFormat;

    // If audio data is sent at a regular rate, but the amount is too much/insufficient to keep the buffer from
    // over/underflowing, stretch the incoming audio data by this factor to compensate
    // In Libretro cores, this factor compensates for the emulation rate differing from the console's native framerate
    // if using VSync, for example
    // The ratio is hostFPS / coreFPS
    qreal audioRatio{ 1.0 };

    // Video

    qreal videoAspectRatio{ 1.0 };
    size_t videoBytesPerLine{ 0 };
    size_t videoBytesPerPixel{ 0 };
    // FIXME: Default to 0
    qreal videoFramerate{ 60.0 };
    VideoRendererType videoMode{ SOFTWARERENDER };
    // FIXME: Default to invalid
    QImage::Format videoPixelFormat{ QImage::Format_RGB16 };
    QSize videoSize;
};
Q_DECLARE_METATYPE( size_t )
Q_DECLARE_METATYPE( ProducerFormat )
