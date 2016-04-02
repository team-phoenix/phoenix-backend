#include "consumer.h"
#include "logging.h"

#include <QElapsedTimer>

Consumer::Consumer():
    consumerFmt() {
}

Consumer::~Consumer() {
}

void Consumer::printFPSStatistics( int printEvery ) {
    Q_ASSERT( printEvery > 0 );
    static QElapsedTimer timer;
    static double timeElapsedSec = 1.0 / 60.0;
    static double totalPeriod = 0.0;
    static QList<double> framePeriods;

    static int rateLimiter = 0;

    if( timer.isValid() ) {
        timeElapsedSec = ( double )timer.nsecsElapsed() / 1000.0 / 1000.0 / 1000.0;
        totalPeriod += timeElapsedSec;
        framePeriods.append( timeElapsedSec );
        timer.restart();

        if( rateLimiter % printEvery == 0 ) {
            qSort( framePeriods.begin(), framePeriods.end() );
            qCDebug( phxVideo ).nospace() << "Current fps: " << 1 / timeElapsedSec << "fps, "
                                          "Mean fps: " << 1 / ( totalPeriod / printEvery ) << "fps, "
                                          "Median fps: " << 1 / framePeriods[ printEvery / 2 ];
            qCDebug( phxVideo ).nospace() << "  Min fps: " << 1 / framePeriods[ printEvery - 1 ] << "fps, "
                                          "Max fps: " << 1 / framePeriods[ 0 ] << "fps";
            totalPeriod = 0.0;
            framePeriods.clear();
            rateLimiter = 0;
        }
    } else {
        timer.start();
    }

    rateLimiter++;
}
