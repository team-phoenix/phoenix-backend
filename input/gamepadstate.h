#pragma once

#include <QMap>
#include <QObject>

#include "node.h"

#include "SDL.h"
#include "SDL_gamecontroller.h"
#include "SDL_haptic.h"

/*
 * GamepadState is a small struct designed to hold the metadata and states of a single controller made available using
 * SDL2's game controller API. An example of how to use this as a consumer to store input states for all controllers
 * can be found in LibretroCore. An example of how to use this as a consumer and producer to transform controller
 * states can be found in Remapper.
 */

struct GamepadState {
    GamepadState();

    // Uniquely identifies the *type* of controller
    SDL_JoystickGUID GUID;

    // String representation of the GUID
    // Most likely ASCII (convert with to/fromLatin1())
    QString GUIDString;

    // A handle for rumble
    SDL_Haptic *haptic { nullptr };
    int hapticID { -1 };

    // Rumble effect parameters
    SDL_HapticEffect hapticEffect;

    // Friendly name
    QString friendlyName;

    // Button and axis states
    Sint16 axis[ SDL_CONTROLLER_AXIS_MAX ] { 0 };
    Uint8 button[ SDL_CONTROLLER_BUTTON_MAX ] { 0 };

    // For internal use

    int joystickID{ 0 };
    int instanceID{ 0 };
    SDL_GameController *gamecontrollerHandle { nullptr };
    SDL_Joystick *joystickHandle { nullptr };

    QString mappingString;

    // Joystick button and axis states
    // For use by Remapper only. For all other uses, use axis and button from above
    // TODO: Support more than 16 axes, 16 hats and 256 buttons?
    Sint16 joystickAxis[ 16 ] { 0 };
    Uint8 joystickButton[ 256 ] { 0 };
    Uint8 joystickHat[ 16 ] { 0 };
};
Q_DECLARE_METATYPE( GamepadState )
