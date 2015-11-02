#include "looper.h"

Looper::Looper( QObject *parent ) : QObject( parent ) {

}

void Looper::beginLoop( double interval ) {

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
                qDebug() << "Inner loop ran" << innerLoopCounter << "times";
                qDebug() << "averageTimerDelta =" << averageTimerDifference << "ms";
                qDebug() << "timeElapsed =" << timeElapsed << "ms | interval =" << interval << "ms";
                qDebug() << "Difference:" << timeElapsed - interval << " -- " << ( ( timeElapsed - interval ) / interval ) * 100.0 << "%";
            }

            innerLoopCounter = 0;

            emit signalFrame();

            // Reset the frame timer
            timeElapsed = ( double )timer.nsecsElapsed() / 1000.0 / 1000.0;
        }

        timer.start();

        // Running these just once per loop means massive overhead from calling timer.start() so many times unaccounted
        //     for by the timer itself
        // The higher the number of loops, the better the accuracy (too high and the low precision will lead to bad timing)
        // The lower the number of loops, the higher the precision (but the results are less trustworthy)
        // 100 seems to be the sweet spot, testing on Windows and OS X give about +-50us accuracy with this value when
        //     running Looper in a QThread with QThread::TimeCriticalPriority
        for( int i = 0; i < 100; i++ ) {
            innerLoopCounter++;
            QCoreApplication::processEvents();
            QThread::yieldCurrentThread();
        }

        timeElapsed += ( double )timer.nsecsElapsed() / 1000.0 / 1000.0;

    }
}

void Looper::endLoop() {
    qDebug() << "phoenix.looper: endLoop() start";
    running = false;
    qDebug() << "phoenix.looper: endLoop() end";
}
