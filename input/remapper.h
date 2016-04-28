#pragma once

#include <QObject>

#include "node.h"

class Remapper : public Node {
        Q_OBJECT

    public:
        explicit Remapper( Node *parent = 0 );

    signals:

    public slots:
};
