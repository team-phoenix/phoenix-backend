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

bool KeyboardListener::eventFilter( QObject */*watched*/, QEvent *event ) {
    if( event->type() == QEvent::KeyPress ) {
        int key = dynamic_cast<QKeyEvent *>( event )->key();
        emit keyPressed( key );
    }

    if( event->type() == QEvent::KeyRelease ) {
        int key = dynamic_cast<QKeyEvent *>( event )->key();
        emit keyReleased( key );
    }

    return false;
}
