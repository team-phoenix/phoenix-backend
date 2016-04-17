#pragma once
#ifndef CONTROL_H
#define CONTROL_H

// Helper for QML and printing enum values
#include <QObject>

/*
 * Functionality and structures common to all control classes.
 *
 * Control signals must be declared in all subclasses using the CONTROL_SIGNALS macro.
 * Use the CONNECT_CONTROL_CONTROLLABLE() macro to easily connect a Control and Controllable together.
 *
 * Control::State defines the global state machine, along with a set of functions to manipulate that state machine.
 *
 * An example of an implementation of Control can be found in CoreControl.
 */

#define CONTROL_SIGNALS \
    void setState( Control::State state );

#define CONNECT_CONTROL_CONTROLLABLE( control, controllable ) \
    connect( dynamic_cast<QObject *>( control ), SIGNAL( setState( Control::State ) ),\
             dynamic_cast<QObject *>( controllable ), SLOT( setState( Control::State ) ) );

#define CLIST_CONNECT_CONTROL_CONTROLLABLE( control, controllable ) \
    connectionList << connect( dynamic_cast<QObject *>( control ), SIGNAL( setState( Control::State ) ),\
             dynamic_cast<QObject *>( controllable ), SLOT( setState( Control::State ) ) );

class Control {
    public:
        Control();

        // State changers
        virtual void load() = 0;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual void reset() = 0;

        // States
        enum State {
            STOPPED = 0,
            LOADING,
            PAUSED,
            PLAYING,
            UNLOADING
        };
        Q_ENUMS( State )

        // This signal has been commented out because the linker will complain no implementation of them
        // exists for Producer. Remember, this does NOT have the Q_OBJECT macro and is NOT a QObject!

        // Directly set the state
        // void setState( Control::State state );

    protected:
        State state;

};

#endif // CONTROL_H
