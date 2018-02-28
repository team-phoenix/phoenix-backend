#include "libretro.h"
#include "Logging.h"
#include "inputdevice.hpp"
#include "inputdeviceinfo.h"

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

    if (SDL_GameControllerAddMapping(controllerDbData) == -1) {
      throw std::runtime_error(qPrintable(QString("SDL Error: " + QString(SDL_GetError()))));
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

  QString getSDLControllerMapping(SDL_JoystickID joystickID)
  {
    QString result;

    SDL_GameController* gameController = SDL_GameControllerFromInstanceID(joystickID);
    char* rawControllerMappingStr = SDL_GameControllerMapping(gameController);

    result = rawControllerMappingStr;
    SDL_free(rawControllerMappingStr);

    return result;
  }

  QVariantHash getInputMapping(SDL_JoystickID joystickID)
  {
    const QString sdlControllerMapping = getSDLControllerMapping(joystickID);
    QStringList splitMappingStr = sdlControllerMapping.split(",");

    QVariantHash inputMapping;

    if (splitMappingStr.size() > 2) {
      const QString controllerGUID = splitMappingStr.takeFirst();
      const QString controllerName = splitMappingStr.takeFirst();

      for (const QString &item : splitMappingStr) {
        const QStringList splitMapEntry = item.split(":");

        if (splitMapEntry.size() == 2) {
          const QString sdlButtonName = splitMapEntry.first();
          const QString sdlButtonMapValue = splitMapEntry.last();
          inputMapping.insert(sdlButtonName, sdlButtonMapValue);
        }
      }
    }

    return inputMapping;
  }

  QList<InputDeviceInfo> getInputDeviceInfoList()
  {
    QList<InputDeviceInfo> infoList;

    for (auto iter = joystickIDToIndexMap.begin(); iter != joystickIDToIndexMap.end(); ++iter) {

      const SDL_JoystickID joystickID = iter.key();


      const QVariantHash inputMapping = getInputMapping(joystickID);

      InputDeviceInfo deviceInfo(QString(SDL_GameControllerNameForIndex(joystickID))
                                 , joystickID
                                 , inputMapping);

      infoList.append(deviceInfo);
    }

    return infoList;
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
      if (static_cast<int>(port) < MAX_DEVICE_SIZE && static_cast<int>(port) < inputDevices.size()) {
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

    const int joystickIndex = inputDevices.size() - 1;
    qCDebug(phxInput) << "GameController added at index" << joystickID;

    joystickIDToIndexMap[ joystickID ] = joystickIndex;
  }

  void removeController(qint32 joystickID)
  {
    const int index = joystickIDToIndexMap.value(joystickID, -1);

    if (index != -1) {
      inputDevices.removeAt(index);
      joystickIDToIndexMap.remove(joystickID);

      qCDebug(phxInput) << "GameController removed at index" << joystickID;

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
