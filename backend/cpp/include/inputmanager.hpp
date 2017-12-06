#include "libretro.h"
#include "logging.h"
#include "inputdevice.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include <QVector>
#include <QHash>
#include <QFile>

static const int MAX_DEVICE_SIZE = 16;
static const int MAX_RETRO_BUTTONS = RETRO_DEVICE_ID_JOYPAD_R3 + 1;

template<typename KeyboardStateBuffer = QVector<qint16>>
class InputManager_T
{
public:
  InputManager_T()
  {
    keyboardStates.resize(MAX_RETRO_BUTTONS);
    keyboardStates.fill(0x0);

    setControllerMappings();

    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_VIDEO) < 0) {
      const QString message = "Unable to initialize SDL2: " + QString(SDL_GetError());
      throw std::runtime_error(message.toStdString());
    }

  }

  ~InputManager_T()
  {
    SDL_Quit();
  }

  void setControllerMappings()
  {
    QFile controllerDbFile(":/gamecontrollerdb.txt");
    controllerDbFile.open(QIODevice::ReadOnly);
    const QByteArray controllerDbData = controllerDbFile.readAll();

    if (SDL_GameControllerAddMapping(controllerDbData) != 1) {
      throw std::runtime_error(SDL_GetError());
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
    inputDevices.clear();
  }

  int numberOfConnectedControllers() const
  {
    return joystickIDToIndexMap.size();
  }

  void updateControllerStates()
  {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

//      qCDebug(phxInput) << event.cbutton.type
//                        << event.cbutton.which
//                        << event.cbutton.button
//                        << event.cbutton.state;

      switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
          addController(event.cdevice.which);
          break;

        case SDL_CONTROLLERDEVICEREMOVED:
          removeController(event.cdevice.which);
          break;

        case SDL_CONTROLLERAXISMOTION: {
            const int index = joystickIDToIndexMap[event.cbutton.which];
            inputDevices[index].setAnalogState(event.caxis.axis, event.caxis.value);
            break;
          }

        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERBUTTONDOWN: {
            const int index = joystickIDToIndexMap[event.cbutton.which];
            inputDevices[index].setDigitalState(event.cbutton.button, event.cbutton.state);
            break;
          }

        default:
          break;
      }
    }
  }

  qint16 getInputState(unsigned port, unsigned device, unsigned index, unsigned id)
  {
    Q_UNUSED(index);
    qint16 state = 0;

    if (device == RETRO_DEVICE_JOYPAD) {
      if (static_cast<int>(port) < MAX_DEVICE_SIZE) {
        return inputDevices.at(port).getInputStateForRetroID(id);
      }
    }

    return state;
  }

private:
  void addController(qint32 joystickID)
  {
    inputDevices.append(InputDevice());

    SDL_GameController* controller = SDL_GameControllerOpen(joystickID);
    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
//    const SDL_JoystickID id = SDL_JoystickInstanceID(joystick);

    joystickIDToIndexMap[ joystickID ] = inputDevices.size() - 1;
  }

  void removeController(qint32 joystickID)
  {
    const int index = joystickIDToIndexMap.value(joystickID, -1);

    if (index != -1) {
      inputDevices.removeAt(index);
      joystickIDToIndexMap.remove(joystickID);

      SDL_GameController* controller = SDL_GameControllerFromInstanceID(joystickID);
      SDL_GameControllerClose(controller);
    }
  }

private:
  KeyboardStateBuffer keyboardStates;
  QVector<InputDevice> inputDevices;
  QHash<SDL_JoystickID, unsigned> joystickIDToIndexMap;
};

using InputManager = InputManager_T<>;
