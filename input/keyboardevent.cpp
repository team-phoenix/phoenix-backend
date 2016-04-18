#include "keyboardevent.h"

#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>

using namespace Input;

KeyboardEvent::KeyboardEvent( Qt::Key k, bool p )
    : key( k ),
      pressed( p )
{
}

KeyboardEvent::KeyboardEvent( QEvent &e )
    : KeyboardEvent( static_cast<QKeyEvent &>( e ) )
{
}

KeyboardEvent::KeyboardEvent( const QKeyEvent &k )
    : KeyboardEvent( static_cast<Qt::Key>( k.key() )
                     , ( k.type() == QEvent::KeyPress ) )
{

}

QDebug operator <<(QDebug o, const KeyboardEvent &event) {
    QDebugStateSaver state( o );
    Q_UNUSED( state );

    o.nospace() << "KeyboardEvent("
                << QKeySequence{ event.key }.toString() << ","
                << event.pressed << ")";

    return o;
}
