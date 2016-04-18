#include "joystickevent.h"

#include <QDebugStateSaver>

using namespace Input;

JoystickButtonEvent::JoystickButtonEvent(int p, JoystickButton t, qint16 s)
    : port( p ),
      type( t ),
      state( s )
{

}

JoystickAxisEvent::JoystickAxisEvent(int p, JoystickAxis t, qint16 s)
    : port( p ),
      type( t ),
      state( s )
{

}

QDebug operator<< (QDebug o, const JoystickButtonEvent &event) {
    QDebugStateSaver saver( o );
    Q_UNUSED( saver );

    o.nospace() << "JoystickButtonEvent("
                << event.port << ","
                << static_cast<int>( event.type ) << ","
                << event.state << ")";
    return o;
}

QDebug operator<< (QDebug o, const JoystickAxisEvent event) {
    QDebugStateSaver saver( o );
    Q_UNUSED( saver );

    o.nospace() << "JoystickAxisEvent("
                << event.port << ","
                << static_cast<int>( event.type ) << ","
                << event.state << ")";
    return o;
}
