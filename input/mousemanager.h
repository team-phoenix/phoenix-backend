#pragma once

#include "node.h"
#include "mousestate.h"

#include <QMutex>
#include <QPointF>

class KeyboardMouseListener;

class MouseManager : public Node {
        Q_OBJECT
    public:
        MouseManager( QObject *parent = nullptr );
        ~MouseManager() = default;

        void setListener( KeyboardMouseListener *listener );

    public slots:
        void mousePressd( QPointF point, Qt::MouseButtons buttons );
        void mouseReleased( QPointF point, Qt::MouseButtons buttons );
        void mouseMoved( QPointF point, Qt::MouseButtons buttons );

        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

    private:
        KeyboardMouseListener *listener;

        // Circular buffer of mouse states
        MouseState mouseBuffer[ 100 ];
        int mouseBufferIndex { 0 };

        QMutex mutex;
        MouseState mouse;
};
