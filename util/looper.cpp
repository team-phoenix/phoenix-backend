#include "looper.h"
#include "logging.h"

#include <QElapsedTimer>
#include <QCoreApplication>
#include <QDateTime>
#include <QThread>

LooperPrivate::LooperPrivate( QObject *parent ): QObject( parent ) {
}

LooperPrivate::~LooperPrivate() {
}


void LooperPrivate::beginLoop( double interval ) {

    // High-precision elapsed time timer
    QElapsedTimer timer;

    // For debugging
    int outerLoopCounter = 1;
    int innerLoopCounter = 0;
    bool printDebug = false;
    int printEvery = 240;

    // Tracks when it's time to send out the signal
    double timeElapsed = 0.0;

    // Assuming the average core runs at 60.0fps natively, make each sample taken only influence 1/60 of the average
    double alpha = 1.0 / 60.0;
    double averageTimerDifference = 0.0;

    running = true;

    while( running ) {

        if( timeElapsed + averageTimerDifference > interval ) {
            timer.start();

            averageTimerDifference = alpha * ( timeElapsed - interval ) + ( 1.0 - alpha ) * averageTimerDifference;

            outerLoopCounter++;

            if( printDebug && ( outerLoopCounter % printEvery == 0 ) )  {
                // Remember, the stuff printed here are snapshots in time, NOT averages!
                qCDebug( phxControl ) << "Inner loop ran" << innerLoopCounter << "times";
                qCDebug( phxControl ) << "averageTimerDelta =" << averageTimerDifference << "ms";
                qCDebug( phxControl ) << "timeElapsed =" << timeElapsed << "ms | interval =" << interval << "ms";
                qCDebug( phxControl ) << "Difference:" << timeElapsed - interval << " -- " << ( ( timeElapsed - interval ) / interval ) * 100.0 << "%";
            }

            innerLoopCounter = 0;

            emit timeout( QDateTime::currentMSecsSinceEpoch() );

            // Reset the frame timer
            timeElapsed = ( double )timer.nsecsElapsed() / 1000.0 / 1000.0;
        }

        timer.start();

        // Busy wait by processing messages and letting other threads do stuff
        // Running these just once per loop means massive overhead from calling timer.start() so many times unaccounted
        //     for by the timer itself
        // The higher the number of loops, the better the accuracy (too high and the low precision will lead to bad timing)
        // The lower the number of loops, the higher the precision (but the results are less trustworthy)
        // 100 seems to be the sweet spot, testing on Windows and OS X give about +-50us accuracy with this value when
        //     running Looper in a QThread with QThread::TimeCriticalPriority
        // This busy waiting is only necessary if Looper does not share its thread with anything else. Set to 1 if it does.
        for( int i = 0; i < 100; i++ ) {
            innerLoopCounter++;
            // This should be safe as we're the only QObject on this thread
            QCoreApplication::processEvents();
            // QThread::yieldCurrentThread();
        }

        timeElapsed += ( double )timer.nsecsElapsed() / 1000.0 / 1000.0;

    }
}

void LooperPrivate::endLoop() {
    running = false;
}

Looper::Looper( QObject *parent ) : QObject( parent ),
    looper( new LooperPrivate ),
    looperThread( new QThread ) {

    connect( this, &Looper::beginLoop, looper, &LooperPrivate::beginLoop );
    connect( this, &Looper::endLoop, looper, &LooperPrivate::endLoop );
    connect( looper, &LooperPrivate::timeout, this, &Looper::timeout );

    looper->moveToThread( looperThread );
    looper->setObjectName( "Looper (internal)" );
    looper->setParent( looperThread );
    qDebug() << looper->objectName() << " parent: " << looper->parent();
    looperThread->setObjectName( "Looper thread (internal)" );
    looperThread->start();

}

Looper::~Looper() {
    if( looperThread->isRunning() ) {
        emit endLoop();
        looperThread->exit();
        looperThread->wait();
    }
}

void Looper::setState( Control::State state ) {

    // We only care about the transition to or away from PLAYING
    if( ( this->currentState == Control::PLAYING && state != Control::PLAYING ) ||
        ( this->currentState != Control::PLAYING && state == Control::PLAYING ) ) {
        if( state == Control::PLAYING ) {
            emit beginLoop( ( 1.0 / hostFPS ) * 1000.0 );
        } else {
            emit endLoop();
        }
    }

    this->currentState = state;

}

void Looper::libretroSetFramerate( qreal hostFPS ) {
    if( this->hostFPS != hostFPS ) {
        qCDebug( phxControl ).nospace() << "Looper beginning loop at " << hostFPS << "fps";
        emit endLoop();
        emit beginLoop( ( 1.0 / hostFPS ) * 1000.0 );
    }

    this->hostFPS = hostFPS;
}
