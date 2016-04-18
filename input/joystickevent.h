#pragma once

#include <libretro.h>
#include <QMetaType>
#include <QtGlobal>
#include <QString>

namespace Input {

    /* Enums */
    enum class JoystickButton {
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

    enum class JoystickAxis {
        Invalid = -1,
        LeftX,
        LeftY,
        RightX,
        RightY,
        LeftTrigger,
        RightTrigger,
        Max,
    };

    /* Events */
    struct JoystickButtonEvent {
        JoystickButtonEvent() = default;
        JoystickButtonEvent( int p, JoystickButton t, qint16 s );

        int port{ -1 };
        JoystickButton type{ JoystickButton::Invalid };
        qint16 state{ 0 };
    };

    struct JoystickAxisEvent {
        JoystickAxisEvent() = default;
        JoystickAxisEvent( int p, JoystickAxis t, qint16 s );

        int port{ -1 };
        JoystickAxis type{ JoystickAxis::Invalid };
        qint16 state{ 0 };
    };

    /* Global Functions */
    QDebug operator<< ( QDebug o, const JoystickButtonEvent &event );
    QDebug operator<< (QDebug o, const JoystickAxisEvent &event);
    inline QString toString( JoystickButton button ) {
        return [&button] {
            switch( button ) {
            case JoystickButton::B:
                return QStringLiteral( "b" );
            case JoystickButton::Y:
                return QStringLiteral( "y" );
            case JoystickButton::A:
                return QStringLiteral( "a" );
            case JoystickButton::X:
                return QStringLiteral( "x" );

            case JoystickButton::Select:
                return QStringLiteral( "select" );
            case JoystickButton::Start:
                return QStringLiteral( "start" );
            case JoystickButton::Guide:
                return QStringLiteral( "guide" );

            case JoystickButton::Up:
                return QStringLiteral( "dpup" );
            case JoystickButton::Down:
                return QStringLiteral( "dpdown" );
            case JoystickButton::Left:
                return QStringLiteral( "dpleft" );
            case JoystickButton::Right:
                return QStringLiteral( "dpright" );

            case JoystickButton::L:
                return QStringLiteral( "leftshoulder" );
            case JoystickButton::R:
                return QStringLiteral( "rightshoulder" );
            case JoystickButton::L2:
                return QStringLiteral( "leftrigger" );
            case JoystickButton::R2:
                return QStringLiteral( "righttrigger" );
            case JoystickButton::L3:
                return QStringLiteral( "leftclick" );
            case JoystickButton::R3:
                return QStringLiteral( "rightclick" );

            default:
                Q_UNREACHABLE();
            }
        }();
    }
}

Q_DECLARE_METATYPE( Input::JoystickButton )
Q_DECLARE_METATYPE( Input::JoystickAxisEvent )
