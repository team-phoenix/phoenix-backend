#pragma once

#include <QObject>
#include <QPointF>

class QEvent;

#include "node.h"

/*
 * KeyboardListener is a simple object that lives on the main thread and intercepts keystrokes. It emits a signal
 * whenever a key is pressed or released.
 */

// FIXME: Keyboard events will still hit QML even while remapping

class KeyboardListener : public QObject {
        Q_OBJECT

    public:
        KeyboardListener();

        bool eventFilter( QObject *watched, QEvent *event ) override;

    signals:
        void keyPressed( int key );
        void keyReleased( int key );
        void mousePressed( QPointF );
        void mouseReleased( QPointF );
};
