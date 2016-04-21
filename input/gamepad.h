#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <libretro.h>
#include <SDL_gamecontroller.h>

#include <QObject>
#include <QHash>
#include <QMap>

class Gamepad : public QObject {
    Q_OBJECT
    Q_PROPERTY( qint16 a READ a NOTIFY aChanged)
    Q_PROPERTY( qint16 b READ b NOTIFY bChanged)
    Q_PROPERTY( qint16 x READ x NOTIFY xChanged)
    Q_PROPERTY( qint16 y READ y NOTIFY yChanged)

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

    qint16 a() const {
        return m_a;
    }

    qint16 b() const {
        return m_b;
    }

    qint16 x() const
    {
        return m_x;
    }

    qint16 y() const {
        return m_y;
    }

    void update();

    QString mapping() const;

    qint32 id() const;
    bool isOpen() const;

    qint16 buttonState( Button button ) const;
    qint16 axisState( Axis axis ) const;

signals:
    void aChanged();
    void bChanged();
    void xChanged();
    void yChanged();

private:
    SDL_GameController *m_SDLGamepad{ nullptr };
    SDL_Joystick *m_SDLJoystick{ nullptr };

    qint16 m_deadZone{ 5000 };

    QVarLengthArray<qint16, static_cast<int>( Gamepad::Button::Max )> m_buttonStates;
    QVarLengthArray<qint16, static_cast<int>( Gamepad::Axis::Max )> m_axisStates;

    qint32 m_id;

    void updateButtonState( Button button, qint16 state );
    void updateAxisState( Axis axis, qint16 state );

    qint16 m_a{ 0 };
    qint16 m_b{ 0 };
    qint16 m_x{ 0 };
    qint16 m_y{ 0 };

    void setA( qint16 t_state ) {
        m_a = t_state ;
        emit aChanged();
    }

    void setB( qint16 t_state  ) {
        m_b = t_state ;
        emit bChanged();
    }

    void setX( qint16 t_state  ) {
        m_x = t_state ;
        emit xChanged();
    }

    void setY( qint16 t_state  ) {
        m_y = t_state ;
        emit yChanged();
    }

};

Gamepad::Button toGamepadButton( int button );
Gamepad::Axis toGamepadAxis( int axis );

#endif // GAMEPAD_H
