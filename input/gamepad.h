#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <libretro.h>
#include <SDL_gamecontroller.h>

#include <QObject>
#include <QHash>
#include <QMap>

class Gamepad : public QObject {
    Q_OBJECT
public:
    enum class Button {
        Invalid = -1,
        B = RETRO_DEVICE_ID_JOYPAD_B,
        Y = RETRO_DEVICE_ID_JOYPAD_Y,
        Select = RETRO_DEVICE_ID_JOYPAD_SELECT,
        Start = RETRO_DEVICE_ID_JOYPAD_START,
        Up = RETRO_DEVICE_ID_JOYPAD_UP,
        Down = RETRO_DEVICE_ID_JOYPAD_DOWN,
        Left = RETRO_DEVICE_ID_JOYPAD_LEFT,
        Right = RETRO_DEVICE_ID_JOYPAD_RIGHT,
        A = RETRO_DEVICE_ID_JOYPAD_A,
        X = RETRO_DEVICE_ID_JOYPAD_X,
        L = RETRO_DEVICE_ID_JOYPAD_L,
        R = RETRO_DEVICE_ID_JOYPAD_R,
        L2 = RETRO_DEVICE_ID_JOYPAD_L2,
        R2 = RETRO_DEVICE_ID_JOYPAD_R2,
        L3 = RETRO_DEVICE_ID_JOYPAD_L3,
        R3 = RETRO_DEVICE_ID_JOYPAD_R3,
        Guide,
        Max,
    };
    Q_ENUM( Button )

    enum class Axis {
        Invalid = -1,
        LeftX,
        LeftY,
        RightX,
        RightY,
        LeftTrigger,
        RightTrigger,
        Max,
    };
    Q_ENUM( Axis)

    Gamepad( int index );
    ~Gamepad();

    void update();

    QMap<QString,QString> mapping() const;
    QString name() const;

    qint32 id() const;
    bool isOpen() const;

    qint16 buttonState( Button button ) const;
    qint16 axisState( Axis axis ) const;


    void setMapping( QVariantMap t_mapping );


private:
    SDL_GameController *m_SDLGamepad{ nullptr };
    SDL_Joystick *m_SDLJoystick{ nullptr };

    qint16 m_deadZone{ 5000 };

    QVarLengthArray<qint16, static_cast<int>( Gamepad::Button::Max )> m_buttonStates;
    QVarLengthArray<qint16, static_cast<int>( Gamepad::Axis::Max )> m_axisStates;

    qint32 m_id;
    QString m_name;
    bool m_isKeyboard;

    void updateButtonState( Button button, qint16 state );
    void updateAxisState( Axis axis, qint16 state );

};


Gamepad::Button toGamepadButton( int button );
Gamepad::Axis toGamepadAxis( int axis );
QString toString( Gamepad::Button button );

#endif // GAMEPAD_H
