#pragma once

#include <QPointF>

struct TouchState
{
    TouchState() = default;
    ~TouchState() = default;

    QPointF point;
    bool pressed{ false };
};

