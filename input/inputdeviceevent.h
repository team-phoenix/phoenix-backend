#ifndef INPUTDEVICEEVENT
#define INPUTDEVICEEVENT

#include <QObject>
#include <QString>
#include <QDebug>

#include "libretro.h"

class InputDevice;

class InputDeviceEvent : public QObject {
        Q_OBJECT

    public:

        enum Event {
            Guide = -1,
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
            Unknown = RETRO_DEVICE_ID_JOYPAD_R3 + 1,
        };

        Q_ENUMS( Event )

        static QString toString( const InputDeviceEvent::Event &event );

        static Event toEvent( const QString button );

};

Q_DECLARE_METATYPE( InputDeviceEvent::Event )

#endif // INPUTDEVICEEVENT

