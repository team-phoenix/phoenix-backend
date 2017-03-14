#pragma once

#include <SDL_gamecontroller.h>

#include <QVector>

class Gamepad {
public:
    explicit Gamepad( int t_joystickIndex );
    ~Gamepad();

public: // Getters

    quint8 getButtonState( uint t_libretroID ) const;
    quint8 getButtonState( SDL_GameControllerButton t_button ) const;

    qint16 getAxisState( SDL_GameControllerAxis t_axis );

    const QString &name() const;
    int instanceID() const;
    bool isAttached() const;

public: // Modifiers

    void updateStates();

private:

    bool open( int t_joystickIndex );

private:
    SDL_GameController *m_sdlController;
    SDL_Joystick *m_sdlJoystick;

    QVector<quint8> m_buttonStates;
    QVector<qint16> m_axisStates;

    QVector<SDL_GameControllerButton> m_buttonMapping;

    QString m_name;
};

