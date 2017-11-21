#include "catch.hpp"

#include "inputmanager.hpp"

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
