#pragma once

#include <QMap>
#include <QObject>

#include "node.h"

#include "SDL.h"
#include "SDL_gamecontroller.h"

/*
 * Gamepad is a small struct designed to hold the metadata and states of a single controller made available using
 * SDL2's game controller API. An example of how to use this as a consumer to store input states for all controllers
 * can be found in LibretroCore. An example of how to use this as a consumer and producer to transform controller
 * states can be found in Remapper.
 */

struct Gamepad {
    Gamepad() = default;
    // Uniquely identifies the *type* of controller
    SDL_JoystickGUID GUID;

    // Friendly name
    QString friendlyName;

    // For internal use
    int joystickID{ 0 };
    int instanceID{ 0 };

    bool isKeyboard{ false };
    Sint16 axis[ SDL_CONTROLLER_AXIS_MAX ] { 0 };
    Uint8 button[ SDL_CONTROLLER_BUTTON_MAX ] { 0 };
};
Q_DECLARE_METATYPE( Gamepad )
