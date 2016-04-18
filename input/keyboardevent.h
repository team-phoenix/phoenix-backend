#pragma once

#include <QMetaType>
#include <QDebug>

class QKeyEvent;
class QEvent;

namespace Input {

    struct KeyboardEvent
    {
    public:
        KeyboardEvent() = default;
        KeyboardEvent( Qt::Key key, bool state );
        KeyboardEvent( QEvent &event );
        KeyboardEvent( const QKeyEvent &keyEvent );

        Qt::Key key{ Qt::Key_unknown };
        bool pressed{ false };
    };

    QDebug operator <<( QDebug o, const KeyboardEvent &event );

}

Q_DECLARE_METATYPE( Input::KeyboardEvent )
