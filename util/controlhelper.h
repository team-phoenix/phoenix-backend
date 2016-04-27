#pragma once

#include "backendcommon.h"

/*
 * Control cannot be a QObject, so this helper class was created so that the enumeration could be used within QML.
 */

class ControlHelper : public QObject {
        Q_OBJECT

    public:
        // States
        // WARNING: This must *exactly* match Control::State!
        enum State {
            STOPPED = 0,
            LOADING,
            PAUSED,
            PLAYING,
            UNLOADING
        };
        Q_ENUM( State )
};
