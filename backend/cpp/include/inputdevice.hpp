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
//    if (id == RETRO_DEVICE_ID_JOYPAD_L2) {
//      return analogStates.at(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
//    } else if (id == RETRO_DEVICE_ID_JOYPAD_R2) {
//      return analogStates.at(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
//    }

    return digitalStates.at(id);
  }

  void setDigitalState(quint8 button, quint8 state)
  {
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
