#pragma once

#include <QtGlobal>
#include <QVector>

#include "libretro.h"
#include <SDL2/SDL.h>

class InputDevice
{
public:
  InputDevice()
    : digitalStates(QVector<qint16>(SDL_CONTROLLER_BUTTON_MAX, 0x0)),
      analogStates(QVector<qint16>(SDL_CONTROLLER_AXIS_MAX, 0x0))
  {
  }

  qint16 getInputStateForRetroID(unsigned id) const
  {

    int event = id;

    switch (id) {
      case RETRO_DEVICE_ID_JOYPAD_A:
        event = SDL_CONTROLLER_BUTTON_B;
        break;

      case RETRO_DEVICE_ID_JOYPAD_B:
        event = SDL_CONTROLLER_BUTTON_A;
        break;

      case RETRO_DEVICE_ID_JOYPAD_X:
        event = SDL_CONTROLLER_BUTTON_Y;
        break;

      case RETRO_DEVICE_ID_JOYPAD_Y:
        event = SDL_CONTROLLER_BUTTON_X;
        break;

      case RETRO_DEVICE_ID_JOYPAD_SELECT:
        event = SDL_CONTROLLER_BUTTON_BACK;
        break;

      case RETRO_DEVICE_ID_JOYPAD_START:
        event = SDL_CONTROLLER_BUTTON_START;
        break;

      case RETRO_DEVICE_ID_JOYPAD_UP:
        event = SDL_CONTROLLER_BUTTON_DPAD_UP;
        break;

      case RETRO_DEVICE_ID_JOYPAD_DOWN:
        event = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        break;

      case RETRO_DEVICE_ID_JOYPAD_LEFT:
        event = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        break;

      case RETRO_DEVICE_ID_JOYPAD_RIGHT:
        event = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        break;

      case RETRO_DEVICE_ID_JOYPAD_L:
        event = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        break;

      case RETRO_DEVICE_ID_JOYPAD_R:
        event = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        break;

      case RETRO_DEVICE_ID_JOYPAD_L3:
        event = SDL_CONTROLLER_BUTTON_LEFTSTICK;
        break;

      case RETRO_DEVICE_ID_JOYPAD_R3:
        event = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        break;

      case RETRO_DEVICE_ID_JOYPAD_L2:
        return analogStates.at(SDL_CONTROLLER_AXIS_TRIGGERLEFT);

      case RETRO_DEVICE_ID_JOYPAD_R2:
        return analogStates.at(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

      default:
        break;
    }

    return digitalStates.at(event);
  }

  void setDigitalState(quint8 button, quint8 state)
  {
    qDebug() << button << state;
    digitalStates[button] = state;
  }

  void setAnalogState(quint8 axis, qint16 state)
  {
    analogStates[axis] = state;
  }

private:
  QVector<qint16> digitalStates;
  QVector<qint16> analogStates;
};
