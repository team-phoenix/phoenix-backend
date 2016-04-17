#pragma once
#ifndef CONTROLLABLE_H
#define CONTROLLABLE_H

#include "control.h"

/*
 * Functionality and structures common to all classes that can be controlled by a controller. Note that this has nothing
 * to do with game controllers!
 *
 * Use the defines provided here in your subclass if you don't need a custom implementation. See "control.h" for a
 * handy macro to connect these slots to Control signals
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
