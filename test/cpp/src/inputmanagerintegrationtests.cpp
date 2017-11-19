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
