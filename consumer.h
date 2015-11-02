#ifndef CONSUMER_H
#define CONSUMER_H

#include "backendcommon.h"

#include "producer.h"

/*
 * Functionality and structures common to all consumers.
 *
 * To connect to these slots:
 * connect( dynamic_cast<QObject *>( ConsumerSubclassPtr ), SIGNAL( producerSignal ),
 *          dynamic_cast<QObject *>( ProducerSubclassPtr ), SLOT( consumerSlot ) );
 *
 * Thanks to peppe and thiago from #Qt on Freenode for the idea
 */

class Consumer {

    public:
        Consumer();
        ~Consumer();

    signals:

    public slots:
        // Information about the type of data to expect
        virtual void consumerFormat( ProducerFormat format ) = 0;

        // Must obtain a mutex to access the data. Only hold onto the mutex long enough to make a copy
        // Type can be one of the following: "audio", "video", "input"
        virtual void consumerData( QString type, QMutex *mutex, void *data, size_t bytes ) = 0;

    protected:
        ProducerFormat format;

};

#endif // CONSUMER_H
