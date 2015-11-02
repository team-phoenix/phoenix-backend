#ifndef LOOPER_H
#define LOOPER_H

#include "backendcommon.h"

class Looper : public QObject {
        Q_OBJECT
    public:
        explicit Looper( QObject *parent = 0 );

    signals:
        void signalFrame();

    public slots:
        // Start the loop. 'interval' is in ms
        void beginLoop( double interval );
        void endLoop();

    private:
        bool running;

};

#endif // LOOPER_H
