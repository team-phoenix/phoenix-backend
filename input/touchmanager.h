#pragma once

#include "node.h"
#include "touchstate.h"

#include <QPointF>
#include <QMutex>

class KeyboardListener;

class TouchManager : public Node
{
    Q_OBJECT
public:
    TouchManager( QObject *parent = nullptr );
    ~TouchManager() = default;

    void setListener( KeyboardListener *t_listener );

public slots:
    void handleMousePress( QPointF t_point );
    void handleMouseRelease( QPointF t_point );

    void commandIn( Command command, QVariant data, qint64 timeStamp ) override;


private:
    KeyboardListener* m_listener;

    QMutex m_mutex;
    TouchState m_touchState;
};
