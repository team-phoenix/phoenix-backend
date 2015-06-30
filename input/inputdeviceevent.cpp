#include "inputdeviceevent.h"


QString InputDeviceEvent::toString( const InputDeviceEvent::Event &event ) {
    switch( event ) {
        case B:
            return "b";

        case A:
            return "a";

        case X:
            return "x";

        case Y:
            return "y";

        case Start:
            return "start";

        case Select:
            return "back";

        case Up:
            return "dup";

        case Down:
            return "dpdown";

        case Left:
            return "dpleft";

        case Right:
            return "dpright";

        case L:
            return "leftshoulder";

        case R:
            return "rightshoulder";

        case L2:
            return "lefttrigger";

        case R2:
            return "righttrigger";

        case L3:
            return "leftstick";

        case R3:
            return "rightstick";

        default:
            return "unknown";
    }
}

InputDeviceEvent::Event InputDeviceEvent::toEvent( const QString button ) {
    if( button == "b" ) {
        return Event::B;
    }

    else if( button == "a" ) {
        return Event::A;
    }

    else if( button == "x" ) {
        return Event::X;
    }

    else if( button == "y" ) {
        return Event::Y;
    }

    else if( button == "start" ) {
        return Event::Start;
    }

    else if( button == "select" ) {
        return Event::Select;
    }

    else if( button == "dpup" ) {
        return Event::Up;
    }

    else if( button == "dpdown" ) {
        return Event::Down;
    }

    else if( button == "dpleft" ) {
        return Event::Left;
    }

    else if( button == "dpright" ) {
        return Event::Right;
    }

    else if( button == "leftshoulder" ) {
        return Event::L;
    }

    else if( button == "rightshoulder" ) {
        return Event::R;
    }

    else if( button == "righttrigger" ) {
        return Event::R2;
    }

    else if( button == "lefttrigger" ) {
        return Event::L2;
    }

    else if( button == "rightstick" ) {
        return Event::R3;
    }

    else if( button == "leftstick" ) {
        return Event::L3;
    }

    return Event::Unknown;
}
