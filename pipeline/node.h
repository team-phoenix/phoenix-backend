#pragma once

#include <QObject>

/*
 * A node in the pipeline tree. Defines a set of signals and slots common to each node.
 */

class Node : public QObject {
        Q_OBJECT

    public:
        explicit Node( QObject *parent = 0 );

    signals:
        void dataOut();
        void controlOut();

    public slots:
        virtual void dataIn();
        virtual void controlIn();


};
