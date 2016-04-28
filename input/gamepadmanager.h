#pragma once

#include <QObject>

#include "node.h"

class GamepadManager : public Node {
        Q_OBJECT

    public:
        explicit GamepadManager( Node *parent = 0 );

    signals:

    public slots:
};
