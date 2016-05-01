#pragma once

#include <QObject>

#include "node.h"

/*
 * Remapper is a Node whose job is to filter and transform input data that passes through it based on stored remapping
 * data. It also provides signals and slots to communicate with a QML-based frontend that will present the user with a
 * graphical interface for setting remappings.
 */

class Remapper : public Node {
        Q_OBJECT

    public:
        explicit Remapper( Node *parent = 0 );

    signals:

    public slots:
};
