#ifndef LOOPER_H
#define LOOPER_H

#include "controllable.h"
#include "pipelinenode.h"

#include <QObject>
#include <QTimer>

/*
 * An ultra-high precision timer that offers far more granularity than anything Qt has to offer. Tests have shown it to
 * be as accurate as +-50us to its target. The drawback, however, is that using this will cause high CPU usage.
 */

class LooperPrivate : public QObject {
        Q_OBJECT

    public:
        explicit LooperPrivate( QObject *parent = 0 );
        ~LooperPrivate();

    signals:
        void timeout( qint64 timestamp );

    public slots:
        // Start the loop. 'interval' is in ms
        void beginLoop( double interval );
        void endLoop();

    private:
        bool running;

};

class Looper : public QObject, public Controllable {
        Q_OBJECT
        PHX_PIPELINE_INTERFACE( Looper )
    public:
        explicit Looper( QObject *parent = 0 );
        ~Looper();

    public slots:

        void dataIn( PipelineNode::DataReason t_reason
                     , QMutex *t_mutex
                     , void *t_data
                     , size_t t_bytes
                     , qint64 t_timestamp );

        // Control
        void setState( Control::State currentState ) override;

        // If Looper is used with a LibretroCore, it's meant to drive the core at its native framerate. This will set it.
        void libretroSetFramerate( qreal hostFPS );

    signals:
        void beginLoop( double interval );
        void endLoop();
        void timeout( qint64 timestamp );

    private:
        LooperPrivate *looper;
        QThread *looperThread;
        qreal hostFPS;

        QTimer m_timer;

};

#endif // LOOPER_H
