#ifndef CONTROLLABLE_H
#define CONTROLLABLE_H

#include "backendcommon.h"

#include "control.h"

/*
 * Functionality and structures common to all classes that can be controlled by a controller. Note that this has nothing
 * to do with game controllers!
 */

#define CONTROLLABLE_SLOT_SETSTATE_DEFAULT \
    void setState( Control::State state ) override { this->currentState = state; }

#define CONTROLLABLE_SLOTS_ABSTRACT \
    virtual void setState( Control::State state ) override = 0;

class Controllable {

    public:
        Controllable();

        // Assigns the state. You may want to do something special when changing to PLAYING or UNLOADING
        virtual void setState( Control::State currentState ) = 0;

    protected:
        Control::State currentState;

};

#endif // CONTROLLABLE_H
