#include "doctest.hpp"

#include "game.hpp"

#include <QByteArray>

SCENARIO("A game file can respond to valid and invalid game paths")
{
  GIVEN("A game with mocked out dependencies") {

    static bool gameExists = true;
    static bool bufferIsEmpty = true;
    static int gameBufferSize = 0;

    struct MockGameFile {
      bool exists() const
      {
        if (gameExists) {
          gameBufferSize = 1;
        } else {
          gameBufferSize = 0;
        }

        return gameExists;
      }
      void setFileName(QString) {}
      bool open(QIODevice::OpenModeFlag) { return true; }
      void close() {}
      QByteArray readAll() { return QByteArray(); }
    };


    struct MockGameBuffer {
      void clear() { bufferIsEmpty = true; gameBufferSize = 0; }
      bool isEmpty() { return bufferIsEmpty; }
      void operator=(QByteArray) { bufferIsEmpty = false; }
      int size() const { return gameBufferSize; }
    };

    static const char* gameFilePath = nullptr;

    struct MockGameFilePath {
      void operator=(QByteArray) { gameFilePath = "/valid/path"; }
      void clear() { gameFilePath = nullptr; }
      const char* constData() const {return gameFilePath;}

    };

    Game_T<MockGameFile, MockGameBuffer, MockGameFilePath> subject;

    WHEN("open() is called with a valid path") {
      REQUIRE_NOTHROW(subject.open("/valid/path/to/game"));

      THEN("copyToRam() pushes the game file data into the game buffer") {
        subject.copyToRam();
        REQUIRE_FALSE(bufferIsEmpty);
      }

      THEN("filePath() returns a valid path") {
        REQUIRE(QString(subject.filePath()) == QString("/valid/path"));
      }

      THEN("clear() empties the game and filePath buffers") {
        subject.clear();
        REQUIRE(bufferIsEmpty == true);
        REQUIRE(gameFilePath == nullptr);
      }
    }

    WHEN("open() is called with a bad path") {
      bufferIsEmpty = true;
      gameExists = false;
      gameFilePath = nullptr;
      REQUIRE_THROWS_AS(subject.open("/bad/path/to/game"), std::runtime_error);

      THEN("copyToRam() throws a runtime error") {
        REQUIRE_THROWS_AS(subject.copyToRam(), std::runtime_error);
      }

      THEN("filePath() returns an invalid path") {
        REQUIRE(subject.filePath() == nullptr);
      }
    }
  }
}
