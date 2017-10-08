#pragma once

#include "gamepad.h"
#include "fakeit.hpp"

#include <QObject>


class Test_Gamepad : public QObject {
    Q_OBJECT
    Gamepad *gamepad;

private slots:

    void init() {
        gamepad = new Gamepad;
    }

    void cleanup() {
        delete gamepad;
    }

    void should_throwRuntimeErrorForAnInvalidJoystickID() {

        const int invalidID = 99999;

        QVERIFY_EXCEPTION_THROWN( gamepad->open( invalidID ), std::runtime_error );

    }

    void should_returnFalseForInvalidJoystickID() {

        const int invalidID = 99999;

        QVERIFY_EXCEPTION_THROWN( gamepad->open( invalidID ), std::runtime_error );

    }

};
