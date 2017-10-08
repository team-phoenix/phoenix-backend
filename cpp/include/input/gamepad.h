#pragma once

#include <SDL_gamecontroller.h>
#include "testing.h"

#include <QVector>

class Gamepad {
    friend class Test_Gamepad;
public:
    explicit Gamepad();
    MOCKABLE ~Gamepad();

public: // Getters

    MOCKABLE quint8 getButtonState( uint t_libretroID ) const;
    MOCKABLE quint8 getButtonState( SDL_GameControllerButton t_button ) const;

    MOCKABLE qint16 getAxisState( SDL_GameControllerAxis t_axis );

    MOCKABLE const QString &name() const;
    MOCKABLE int instanceID() const;
    MOCKABLE bool isAttached() const;

public: // Modifiers

    MOCKABLE void updateStates();

    MOCKABLE void open( int t_joystickIndex );

private:
    SDL_GameController *m_sdlController;
    SDL_Joystick *m_sdlJoystick;

    QVector<quint8> m_buttonStates;
    QVector<qint16> m_axisStates;

    QVector<SDL_GameControllerButton> m_buttonMapping;

    QString m_name;
};

