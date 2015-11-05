#ifndef CONTROL_H
#define CONTROL_H

#include "backendcommon.h"

// Helper for QML and printing enum values
#include "controlhelper.h"

/*
 * Functionality and structures common to all control classes.
 *
 * Control signals must be declared in all subclasses using the CONTROL_SIGNALS macro.
 * Use the CONNECT_CONTROL_CONTROLLABLE() macro to easily connect a Control and Controllable together.
 */

#define CONTROL_SIGNALS \
    void setState( Control::State state );

#define CONNECT_CONTROL_CONTROLLABLE( control, controllable ) \
    connect( dynamic_cast<QObject *>( control ), SIGNAL( setState( Control::State ) ),\
             dynamic_cast<QObject *>( controllable ), SLOT( setState( Control::State ) ) );\
    connect( dynamic_cast<QObject *>( control ), SIGNAL( setFramerate( qreal ) ),\
             dynamic_cast<QObject *>( controllable ), SLOT( setFramerate( qreal ) ) )

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
            INIT = 0,
            STOPPED,
            LOADING,
            PAUSED,
            PLAYING,
            UNLOADING
        };
        Q_ENUMS( State )

    signals:
        // Directly set the state
        // void setState( Control::State state );

        // Inform everyone what the framerate driving control signals is
        // void setFramerate( qreal framerate );

    protected:
        State state;

    public slots:
};

#endif // CONTROL_H
