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

SCENARIO("The input manager has working defaults to fit 16 gamepads")
{
  GIVEN("a real input manager") {

    InputManager subject;
    WHEN("InputManager() constructor is called") {

      THEN("the gamepad buffer holds 16 controller states") {
        REQUIRE(subject.getControllerStates().size() == 16);
      }

      THEN("controller states are of size 16") {
        auto &allGamepadStates = subject.getControllerStates();

        for (const QVector<qint16> &states : allGamepadStates) {
          REQUIRE(states.size() == 16);
        }
      }

      THEN("the gamepad buffer states are zeroed out") {
        auto &allGamepadStates = subject.getControllerStates();

        for (const QVector<qint16> &states : allGamepadStates) {
          for (qint16 s : states) {
            REQUIRE(s == 0);
          }
        }
      }
    }
  }
}

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

SCENARIO("The user has their gamepad buffer updated when there are new input events")
{
  GIVEN("a real input manager") {

    InputManager subject;

    const SDL_JoystickID which = 0;

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
      pushEvents(sdlButtonDownEvents);
      subject.updateControllerStates();

      THEN("the gamepad states will be all set to SDL_PRESSED") {

        // TODO - Fix that last state, 15, the push events stuff only pushed up to event 14

        for (int i = RETRO_DEVICE_ID_JOYPAD_B; i < RETRO_DEVICE_ID_JOYPAD_R3 + 1; ++i) {
          qint16 state = subject.getInputState(which, RETRO_DEVICE_JOYPAD, 0, i);
          REQUIRE(state == SDL_PRESSED);
        }

      }
    }

    WHEN("all button down events are fired") {
      pushEvents(sdlButtonDownEvents);
      pushEvents(sdlButtonUpEvents);
      subject.updateControllerStates();

      THEN("the gamepad states will be all set to SDL_RELEASED") {

        // TODO - Fix that last state, 15, the push events stuff only pushed up to event 14

        for (int i = RETRO_DEVICE_ID_JOYPAD_B; i < RETRO_DEVICE_ID_JOYPAD_R3 + 1; ++i) {
          qint16 state = subject.getInputState(which, RETRO_DEVICE_JOYPAD, 0, i);
          REQUIRE(state == SDL_RELEASED);
        }

      }
    }
  }

}
