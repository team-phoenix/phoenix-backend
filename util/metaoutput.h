#pragma once

#include <QObject>

#include "node.h"

/*
 * MetaOutput is a Node that acknowledges state changes from its parent (a subclass of Core). In addition, it exposes
 * properties to QML that help QML decide what buttons should be available to
 * the user
 */
class MetaOutput : public QObject {
        Q_OBJECT

    public:
        explicit MetaOutput( QObject *parent = 0 );

    signals:

    public slots:
};
