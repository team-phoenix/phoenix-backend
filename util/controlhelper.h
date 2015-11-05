#ifndef CONTROLQMLENUM_H
#define CONTROLQMLENUM_H

#include "backendcommon.h"

class ControlHelper : public QObject {
        Q_OBJECT
    public:
        // States
        enum State {
            INIT = 0,
            STOPPED,
            LOADING,
            PAUSED,
            PLAYING,
            UNLOADING
        };
        Q_ENUM( State )
};

#endif // CONTROLQMLENUM_H

