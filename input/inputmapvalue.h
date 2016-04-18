#pragma once

#include "libretro_cpp.h"
#include <QString>

namespace Input {

    struct InputMapValue {
        InputMapValue() = default;
        Libretro::Gamepad::DigitalButtons key;
        QString label;
    };

    uint qHash( const InputMapValue &value, uint seed );

}

