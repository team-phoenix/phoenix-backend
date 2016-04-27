#pragma once

#include <QObject>

class GlobalGamepad : public QObject {
        Q_OBJECT

    public:
        explicit GlobalGamepad( QObject *parent = 0 );

    signals:

    public slots:
};
