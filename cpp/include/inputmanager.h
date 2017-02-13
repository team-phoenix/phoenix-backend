#pragma once

#include <QObject>

//class InputDevice {

//public:

//    InputDevice()
//    //explicit InputDevice( QObject *parent = nullptr );
//private:

//};

class InputManager : public QObject
{
    Q_OBJECT
public:
    explicit InputManager( QObject *parent = nullptr );

public slots:
    void poll();
};
