#pragma once

#include <libretro.h>

namespace Libretro {

    namespace Gamepad {

        enum class DigitalButtons {
            // Action Buttons
            B = RETRO_DEVICE_ID_JOYPAD_B,
            Y = RETRO_DEVICE_ID_JOYPAD_Y,
            A = RETRO_DEVICE_ID_JOYPAD_A,
            X = RETRO_DEVICE_ID_JOYPAD_X,

            // Menu / Inventory Buttons
            Select = RETRO_DEVICE_ID_JOYPAD_SELECT,
            Start = RETRO_DEVICE_ID_JOYPAD_START,

            // Digital Directional Buttons
            Up = RETRO_DEVICE_ID_JOYPAD_UP,
            Down = RETRO_DEVICE_ID_JOYPAD_DOWN,
            Left = RETRO_DEVICE_ID_JOYPAD_LEFT,
            Right = RETRO_DEVICE_ID_JOYPAD_RIGHT,

            // Trigger Buttons
            L = RETRO_DEVICE_ID_JOYPAD_L,
            R = RETRO_DEVICE_ID_JOYPAD_R,
            L2 = RETRO_DEVICE_ID_JOYPAD_L2,
            R2 = RETRO_DEVICE_ID_JOYPAD_R2,

            // Joystick Click Buttons
            L3 = RETRO_DEVICE_ID_JOYPAD_L3,
            R3 = RETRO_DEVICE_ID_JOYPAD_R3,

            First = B,
            Last = R3,
        };

        enum class AnalogButtons {
            Left = RETRO_DEVICE_INDEX_ANALOG_LEFT,
            Right = RETRO_DEVICE_INDEX_ANALOG_RIGHT,
            X = RETRO_DEVICE_ID_ANALOG_X,
            Y = RETRO_DEVICE_ID_ANALOG_Y,

            First = Left,
            Last = Y,
        };

        enum class Type {
            // Doesn't have analog sticks, think Super Nintendo controller.
            Digital = RETRO_DEVICE_JOYPAD,

            // Has analog sticks, think PlayStation controller
            Analog = RETRO_DEVICE_ANALOG,


            First = Digital,
            Last = Analog,
        };

    }


}
