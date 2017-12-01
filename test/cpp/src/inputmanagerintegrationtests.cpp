#include "catch.hpp"

#include "inputmanager.hpp"

#include <SDL2/SDL.h>

SCENARIO("The input manager has working defaults to fit 16 key states")
{
  GIVEN("a real input manager") {

    InputManager subject;
    WHEN("getKeyboardBuffer() is called") {

      qint16* keyboardBuffer = subject.getKeyboardBuffer();
      const int keyboardBufferSize = subject.getKeyboardBufferSize();

      THEN("the keyboard buffer is not null") {
        REQUIRE(keyboardBuffer != nullptr);
      }

      THEN("the keyboard buffer size equals 16") {
        REQUIRE(keyboardBufferSize == RETRO_DEVICE_ID_JOYPAD_R3 + 1);
      }

      THEN("The keyboard states are all zeroed out") {
        for (int i = 0; i < keyboardBufferSize; ++i) {
          REQUIRE(keyboardBuffer[i] == 0x0);
        }
      }
    }
  }
}

//SCENARIO("The input manager has working defaults to fit 16 gamepads")
//{
//  GIVEN("a real input manager") {

//    InputManager subject;
//    WHEN("InputManager() constructor is called") {

//      THEN("the gamepad buffer holds 16 controller states") {
//        REQUIRE(subject.getControllerStates().size() == 16);
//      }

//      THEN("controller states are of size 16") {
//        auto &allGamepadStates = subject.getControllerStates();

//        for (const QVector<qint16> &states : allGamepadStates) {
//          REQUIRE(states.size() == 16);
//        }
//      }

//      THEN("the gamepad buffer states are zeroed out") {
//        auto &allGamepadStates = subject.getControllerStates();

//        for (const QVector<qint16> &states : allGamepadStates) {
//          for (qint16 s : states) {
//            REQUIRE(s == 0);
//          }
//        }
//      }
//    }
//  }
//}

inline SDL_Event createControllerButtonEvent(quint32 type, SDL_JoystickID which,
                                             quint8 button,
                                             quint8 state)
{
  SDL_ControllerButtonEvent controllerEvent;
  controllerEvent.type = type;
  controllerEvent.which = which;
  controllerEvent.button = button;
  controllerEvent.state = state;

  SDL_Event event;
  event.cbutton = controllerEvent;
  return event;
}

inline void pushEvents(QList<SDL_Event> &events)
{
  for (SDL_Event &event : events) {
    REQUIRE(SDL_PushEvent(&event) == SDL_TRUE);
  }
}

inline void addController(qint32 id)
{
  SDL_Event addControllerEvent;
  addControllerEvent.cdevice.which = id;
  addControllerEvent.type = SDL_CONTROLLERDEVICEADDED;

  SDL_PushEvent(&addControllerEvent);
}

inline void removeController(qint32 id)
{
  SDL_Event removeControllerEvent;
  removeControllerEvent.cdevice.which = id;
  removeControllerEvent.type = SDL_CONTROLLERDEVICEREMOVED;

  SDL_PushEvent(&removeControllerEvent);
}

SCENARIO("The user has their gamepad buffer updated when there are new input events")
{
  GIVEN("a real input manager") {

    InputManager subject;

    const SDL_JoystickID which = 3;

    QList<SDL_Event> sdlButtonDownEvents;

    for (int i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
      sdlButtonDownEvents.append(createControllerButtonEvent(SDL_CONTROLLERBUTTONDOWN,
                                                             which,
                                                             i, SDL_PRESSED));
    }

    QList<SDL_Event> sdlButtonUpEvents;

    for (int i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
      sdlButtonUpEvents.append(createControllerButtonEvent(SDL_CONTROLLERBUTTONUP,
                                                           which,
                                                           i, SDL_RELEASED));
    }

    WHEN("all button down events are fired") {
      addController(which);
      pushEvents(sdlButtonDownEvents);
      subject.updateControllerStates();

      THEN("the gamepad states will be all set to SDL_PRESSED") {

        // TODO - Fix that last state, 15, the push events stuff only pushed up to event 14

        for (int i = RETRO_DEVICE_ID_JOYPAD_B; i < RETRO_DEVICE_ID_JOYPAD_R3/* + 1*/; ++i) {
          qint16 state = subject.getInputState(0, RETRO_DEVICE_JOYPAD, 0, i);
          REQUIRE(state == SDL_PRESSED);
          qDebug() << i << state;
        }

      }
    }

    WHEN("all button up events are fired") {
      addController(which);
      pushEvents(sdlButtonDownEvents);
      pushEvents(sdlButtonUpEvents);
      subject.updateControllerStates();

      THEN("the gamepad states will be all set to SDL_RELEASED") {

        // TODO - Fix that last state, 15, the push events stuff only pushed up to event 14

        for (int i = RETRO_DEVICE_ID_JOYPAD_B; i < RETRO_DEVICE_ID_JOYPAD_R3/* + 1*/; ++i) {
          qint16 state = subject.getInputState(0, RETRO_DEVICE_JOYPAD, 0, i);
          REQUIRE(state == SDL_RELEASED);
        }
      }
    }

    WHEN("user can add and remove a gamepad") {
      addController(2);
      subject.updateControllerStates();
      REQUIRE(subject.numberOfConnectedControllers() == 1);

      removeController(2);
      subject.updateControllerStates();
      REQUIRE(subject.numberOfConnectedControllers() == 0);

      removeController(2);
      subject.updateControllerStates();
      REQUIRE(subject.numberOfConnectedControllers() == 0);
    }

//    WHEN("the controller device added event was fired") {
//      SDL_Event addControllerEvent;
//      addControllerEvent.cdevice.which = 0;
//      addControllerEvent.type = SDL_CONTROLLERDEVICEADDED;
//      SDL_PushEvent(&addControllerEvent);

//      subject.updateControllerStates();

//      THEN("the gamecontroller will be added to the list of states") {
//        for (int i = RETRO_DEVICE_ID_JOYPAD_B; i < RETRO_DEVICE_ID_JOYPAD_R3 + 1; ++i) {
//          qint16 state = subject.getInputState(0, RETRO_DEVICE_JOYPAD, 0, i);
//          REQUIRE(state == SDL_RELEASED);
//        }
//      }
//    }

//    WHEN("the controller device added event was fired with an id of 1") {
//      SDL_Event addControllerEvent;
//      addControllerEvent.cdevice.which = 1;
//      addControllerEvent.type = SDL_CONTROLLERDEVICEADDED;

//      SDL_PushEvent(&addControllerEvent);
//      auto buttonADownEvent = createControllerButtonEvent(SDL_CONTROLLERBUTTONDOWN, 1,
//                                                          SDL_CONTROLLER_BUTTON_A, 1);
//      SDL_PushEvent(&buttonADownEvent);

//      subject.updateControllerStates();

//      THEN("the gamecontroller will be added to the list of states") {
//        qint16 state = subject.getInputState(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
//        REQUIRE(state == SDL_PRESSED);
//      }
//    }
  }
}
