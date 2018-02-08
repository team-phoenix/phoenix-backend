#pragma once

#include <QString>
#include <QVariantHash>

#include <SDL2/SDL_gamecontroller.h>

class InputMappingBuilder
{
public:

  static QVariantHash getInputMapping(SDL_JoystickID joystickID)
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

  static QString buildInputMappingString(SDL_JoystickID joystickID,
                                         QString oldButton,
                                         SDL_GameControllerButton newPressedButton)
  {
    const QString newButtonBindString = getButtonBindValue(joystickID, newPressedButton);

    const QString sdlControllerMapping = getSDLControllerMapping(joystickID);
    QStringList splitMappingStr = sdlControllerMapping.split(",");

    QStringList inputMappingStringList;

    if (splitMappingStr.size() > 2) {
      const QString controllerGUID = splitMappingStr.takeFirst();
      const QString controllerName = splitMappingStr.takeFirst();

      inputMappingStringList.append(controllerGUID);
      inputMappingStringList.append(controllerName);

      QVariantHash inputMapping;

      for (const QString &item : splitMappingStr) {
        const QStringList splitMapEntry = item.split(":");

        if (splitMapEntry.size() == 2) {
          const QString sdlButtonName = splitMapEntry.first();
          const QString sdlButtonMapValue = splitMapEntry.last();
          inputMapping.insert(sdlButtonName, sdlButtonMapValue);
        }
      }

      for (auto iter = inputMapping.begin(); iter != inputMapping.end(); ++iter) {
        const QString mappingKey = iter.key();

        if (mappingKey == oldButton) {
          auto debug = qDebug() << "Remapped" << mappingKey << "from" << inputMapping[mappingKey];
          inputMapping[mappingKey] = newButtonBindString;
          debug << "to" << inputMapping[mappingKey];
          break;
        }
      }

//      qDebug() << inputMapping;
    }

    return inputMappingStringList.join(",");
  }

private:

  static QString getSDLControllerMapping(SDL_JoystickID joystickID)
  {
    QString result;

    SDL_GameController* gameController = SDL_GameControllerFromInstanceID(joystickID);
    char* rawControllerMappingStr = SDL_GameControllerMapping(gameController);

    result = rawControllerMappingStr;
    SDL_free(rawControllerMappingStr);

    return result;
  }

  static QString getButtonBindValue(SDL_JoystickID joystickID, SDL_GameControllerButton button)
  {
    SDL_GameController* gameController = SDL_GameControllerFromInstanceID(joystickID);
    const SDL_GameControllerButtonBind buttonBind = SDL_GameControllerGetBindForButton(gameController,
                                                                                       button);

    QString buttonPrefix;

    qDebug() << "bind" << buttonBind.bindType;

    if (buttonBind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON) {
      buttonPrefix = "b";
    } else if (buttonBind.bindType == SDL_CONTROLLER_BINDTYPE_HAT) {
      buttonPrefix = "a0.";
    }

    QString bindValue;

    if (!buttonPrefix.isEmpty()) {
      bindValue = buttonPrefix + QString::number(static_cast<int>(button));
    }

    return bindValue;
  }

  static QString getButtonString(SDL_GameControllerButton button)
  {
    QString result;
    const char* buttonRawString = SDL_GameControllerGetStringForButton(button);

    if (buttonRawString) {
      result = buttonRawString;
    }

    return result;
  }
};
