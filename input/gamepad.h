#pragma once

#include <QMap>
#include <QObject>

#include "node.h"

#include "SDL.h"
#include "SDL_gamecontroller.h"

struct Gamepad {
    bool connected{ true };

    // Uniquely identifies the *type* of controller
    SDL_JoystickGUID GUID;

    // For internal use
    int joystickID{ 0 };
    int instanceID{ 0 };

    bool isKeyboard{ false };
    Sint16 axis[ SDL_CONTROLLER_AXIS_MAX ]{ 0 };
    Uint8 button[ SDL_CONTROLLER_BUTTON_MAX ]{ 0 };
};
