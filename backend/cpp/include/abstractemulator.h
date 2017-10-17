#pragma once

#include <QObject>

class AbstractEmulator : public QObject {
    Q_OBJECT
public:
    explicit AbstractEmulator( QObject *parent = nullptr )
        : QObject( parent ) {

    }

    ~AbstractEmulator() {}

    virtual void init() = 0;
    virtual void fini() = 0;

    virtual void run() = 0;
};
