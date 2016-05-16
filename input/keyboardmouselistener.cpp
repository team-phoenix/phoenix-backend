#include "keyboardmouselistener.h"

#include <QGuiApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QtGlobal>
#include <QWindow>

KeyboardMouseListener::KeyboardMouseListener() {
    Q_ASSERT( QGuiApplication::topLevelWindows().size() > 0 );

    auto *window = QGuiApplication::topLevelWindows().at( 0 );

    Q_CHECK_PTR( window );

    window->installEventFilter( this );
}

bool KeyboardMouseListener::eventFilter( QObject *watched, QEvent *event ) {

    switch( event->type() ) {
        case QEvent::KeyPress: {
            //qDebug() << "press" << static_cast<QKeyEvent *>( event )->text() << static_cast<QKeyEvent *>( event )->modifiers();
            emit keyPressed( static_cast<QKeyEvent *>( event )->key() );
            break;
        }

        case QEvent::KeyRelease: {
            //qDebug() << "release" << static_cast<QKeyEvent *>( event )->text() << static_cast<QKeyEvent *>( event )->modifiers();
            emit keyReleased( static_cast<QKeyEvent *>( event )->key() );
            break;
        }

        case QEvent::MouseButtonPress: {
            emit mousePressed( static_cast<QMouseEvent *>( event )->windowPos(), static_cast<QMouseEvent *>( event )->buttons() );
            break;
        }

        case QEvent::MouseButtonRelease: {
            emit mouseReleased( static_cast<QMouseEvent *>( event )->windowPos(), static_cast<QMouseEvent *>( event )->buttons() );
            break;
        }

        case QEvent::MouseMove: {
            emit mouseMoved( static_cast<QMouseEvent *>( event )->windowPos(), static_cast<QMouseEvent *>( event )->buttons() );
            break;
        }

        default:
            break;
    }

    return QObject::eventFilter( watched, event );
}
