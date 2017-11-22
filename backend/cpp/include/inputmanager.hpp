
#include "libretro.h"

#include <QVector>

#include <SDL2/SDL.h>

#include "logging.h"

template<typename KeyboardStateBuffer = QVector<qint16>>
class InputManager_T
{
public:

  InputManager_T()
  {
    keyboardStates.resize(RETRO_DEVICE_ID_JOYPAD_R3 + 1);
    keyboardStates.fill(0x0);

    gamepadStates = QVector<QVector<qint16>>(16, QVector<qint16>(16, 0x0));

    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_VIDEO) < 0) {
      const QString message = "Unable to initialize SDL2: " + QString(SDL_GetError());
      throw std::runtime_error(message.toStdString());
    }
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

  void updateControllerStates()
  {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      /*
        qCDebug(phxInput) << event.cbutton.type
                          << event.cbutton.which
                          << event.cbutton.button
                          << event.cbutton.state;*/

      switch (event.type) {
        case SDL_CONTROLLERBUTTONDOWN: {
            auto &states = gamepadStates[event.cbutton.which];
            states[event.cbutton.button] = event.cbutton.state;
            break;
          }

        default:
          break;
      }
    }
  }

  const QVector<QVector<qint16>> &getControllerStates() const
  {
    return gamepadStates;
  }

  qint16 getInputState(unsigned port, unsigned device, unsigned index, unsigned id)
  {
    Q_UNUSED(index);
    qint16 state = 0;

    if (device == RETRO_DEVICE_JOYPAD) {
      if (static_cast<int>(port) < gamepadStates.size()) {
        const auto &states = gamepadStates.at(port);
        state = states.at(id);
      }
    }

    return state;
  }

private:
  KeyboardStateBuffer keyboardStates;
  QVector<QVector<qint16>> gamepadStates;
};

using InputManager = InputManager_T<>;
