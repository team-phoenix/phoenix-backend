#include "keyboardlistener.h"

#include <QGuiApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QtGlobal>
#include <QWindow>

KeyboardListener::KeyboardListener() {
    Q_ASSERT( QGuiApplication::topLevelWindows().size() > 0 );

    auto *window = QGuiApplication::topLevelWindows().at( 0 );

    Q_CHECK_PTR( window );

    window->installEventFilter( this );
}

bool KeyboardListener::eventFilter( QObject *watched, QEvent *event ) {

    switch( event->type() ) {
        case QEvent::KeyPress: {
            emit keyPressed( static_cast<QKeyEvent *>( event )->key() );
            event->accept();
            break;
        }
        case QEvent::KeyRelease: {
            emit keyReleased( static_cast<QKeyEvent *>( event )->key() );
            event->accept();
            break;
        }
        case QEvent::MouseButtonPress: {
            emit mousePressed( static_cast<QMouseEvent *>( event )->localPos() );
            event->accept();
            break;
        }
        case QEvent::MouseButtonRelease: {
            emit mouseReleased( static_cast<QMouseEvent *>( event )->localPos() );
            event->accept();
            break;
        }
        default:
            break;
    }

    return QObject::eventFilter( watched, event );
}
