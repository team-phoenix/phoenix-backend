#ifndef CONSUMER_H
#define CONSUMER_H

#include "producer.h"

/*
 * Functionality and structures common to all consumers.
 *
 * To declare these slots in an abstract class you must use the define provided:
 * class AbstractConsumerSubclass : public QObject, public Consumer {
 *     public slots:
 *         CONSUMER_SLOTS_ABSTRACT
 *         void anyOtherSlots();
 * };
 *
 * You can then override them in whatever subclass of the abstract class like normal.
 *
 * To connect to these slots, use the macro provided in "producer.h" or just follow this form:
 * connect( dynamic_cast<QObject *>( ProducerSubclassPtr ), SIGNAL( producerSignal( argType, anotherArgType ) ),
 *          dynamic_cast<QObject *>( ConsumerSubclassPtr ), SLOT( consumerSlot( argType, anotherArgType ) ) );
 *
 * Check out producer.h for more information
 *
 * Thanks to peppe and thiago from #Qt on Freenode for the idea
 */

// In order to properly declare the slots in your abstract subclass of Consumer, simply use this macro under "public slots:"
// Documented below

struct ConsumerFormat {
    ConsumerFormat() = default;
    ~ConsumerFormat() = default;

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
    qreal videoFramerate{ 60.0 };
    VideoRendererType videoMode{ SOFTWARERENDER };
    QImage::Format videoPixelFormat{ QImage::Format_RGB16 };
    QSize videoSize{ 720, 480 };
};

#define CONSUMER_SLOTS_ABSTRACT \
    virtual void consumerFormat( ProducerFormat consumerFmt ) override = 0; \
    virtual void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) override = 0;

class Consumer {

    public:
        Consumer();
        ~Consumer();

        // Information about the type of data to expect
        virtual void consumerFormat( ProducerFormat consumerFmt ) = 0;

        // Must obtain a mutex to access the data. Only hold onto the mutex long enough to make a copy
        // Type can be one of the following: "audio", "video", "input", "touchinput"
        // You can check the timestamp against QDateTime::currentMSecsSinceEpoch() to see how far out of sync you might be
        virtual void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) = 0;

    protected:
        // Data given by the producer
        ProducerFormat consumerFmt;

        // Print statistics every (printEvery) times this function is called
        void printFPSStatistics( int printEvery = 60 );

};

#endif // CONSUMER_H
