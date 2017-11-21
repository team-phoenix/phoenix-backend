
#include "libretro.h"

#include <QVector>

#include <SDL2/SDL.h>

struct ShittyUpdater {

};

template<typename KeyboardStateBuffer = QVector<qint16>,
         typename GamepadStateUpdater = ShittyUpdater>
class InputManager_T
{
public:

  InputManager_T()
  {
    keyboardStates.resize(RETRO_DEVICE_ID_JOYPAD_R3 + 1);
    keyboardStates.fill(0x0);

    gamepadStates = QVector<QVector<qint16>>(16, QVector<qint16>(16, 0x0));
  }

  qint16* getKeyboardBuffer()
  {
    return keyboardStates.data();
  }

  int getKeyboardBufferSize() const
  {
    return keyboardStates.size();
  }

  void clear()
  {
    keyboardStates.clear();
  }

  void pollControllerStates()
  {
    gamepadStateUpdater.updateStates(gamepadStates);
  }

  const QVector<QVector<qint16>> &getControllerStates() const
  {
    return gamepadStates;
  }

private:
  KeyboardStateBuffer keyboardStates;
  QVector<QVector<qint16>> gamepadStates;
  GamepadStateUpdater gamepadStateUpdater;
};

using InputManager = InputManager_T<>;
