#include "catch.hpp"

#include "inputmanager.hpp"

SCENARIO("The input manager is able to get and set the data "
         "inside of the keyboard buffer")
{
  GIVEN("a mocked out keyboard buffer") {

    static qint16 exceptedBuffer[2] = { 0xFF, 0xAA };
    struct MockKeyboardBuffer {
      qint16* data() const { return exceptedBuffer; }
      int size() const { return 1; }

      void resize(int) {}
      void fill(qint16, int size = -1) { Q_UNUSED(size)}
    };

    InputManager_T<MockKeyboardBuffer> subject;

    WHEN("getKeyboardBuffer() is called") {
      qint16* actualBuffer = subject.getKeyboardBuffer();

      THEN("it returns a pointer to the internal keyboard buffer") {
        REQUIRE(actualBuffer == exceptedBuffer);
      }

    }

    WHEN("getKeyboardBufferSize() is called") {
      int actualSize = subject.getKeyboardBufferSize();

      THEN("it returns the size of the buffer") {
        REQUIRE(actualSize == 1);
      }
    }

  }
}

SCENARIO("The input manager can store controller states")
{
  GIVEN("") {

    static QVector<qint16> expectedGamepadStates;

    struct MockKeyboardBuffer {
      void resize(int) {}
      void fill(qint16) {}
    };


    struct MockGamepadUpdater {
      void updateStates(QVector<QVector<qint16>> &buffer)
      {
        buffer = QVector<QVector<qint16>> { {0x01}, {0x01} };
      }
    };

    InputManager_T<MockKeyboardBuffer, MockGamepadUpdater> subject;

    WHEN("pollControllerStates() is called") {

      auto beforeStates = subject.getControllerStates();
      subject.pollControllerStates();
      auto afterStates = subject.getControllerStates();

      THEN("the controller states will have been updated and filled in") {
        REQUIRE(beforeStates != afterStates);
        QVector<QVector<qint16>> expectedBuffer({ {0x01}, {0x01} });
        REQUIRE(afterStates == expectedBuffer);
      }
    }
  }
}
