#pragma once

#include <QObject>

class GamepadManager : public QObject {
        Q_OBJECT

    public:
        explicit GamepadManager( QObject *parent = 0 );

    signals:

    public slots:
};
