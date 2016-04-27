#pragma once

#include <QObject>

class Remapper : public QObject {
        Q_OBJECT

    public:
        explicit Remapper( QObject *parent = 0 );

    signals:

    public slots:
};
