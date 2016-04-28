#pragma once

#include <QObject>

#include "node.h"

class GlobalGamepad : public Node {
        Q_OBJECT

    public:
        explicit GlobalGamepad( Node *parent = 0 );

    signals:

    public slots:
};
