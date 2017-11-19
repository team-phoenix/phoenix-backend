#include "catch.hpp"

#include "game.hpp"

#include <QFile>
#include <QDir>
#include <QString>

SCENARIO("A game can handle opening valid and invalid game paths")
{
  GIVEN("a real game object and a real game file") {

    const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

    const QString workingGamePath = QDir::temp().filePath("tempGame");

    QFile::copy(qrcGamePath, workingGamePath);

    REQUIRE(QFile::exists(workingGamePath) == true);

    QFile workingGame(workingGamePath);
    REQUIRE(workingGame.open(QIODevice::ReadOnly) == true);
    const QByteArray workingGameBuffer = workingGame.readAll();

    Game game;

    WHEN("open() is called with a valid core path") {

      REQUIRE_NOTHROW(game.open(workingGamePath));

      THEN("filePath() returns the workingGamePath") {
        REQUIRE(QString(game.filePath()) == workingGamePath);
      }

      THEN("copyToRam() copies the game data into a buffer") {
        REQUIRE_NOTHROW(game.copyToRam());
        REQUIRE(game.size() == workingGameBuffer.size());

        const QByteArray gameData = QByteArray(game.constData(), game.size());
        REQUIRE(gameData == workingGameBuffer);
      }
    }

    WHEN("open() is called with an invalid core path") {
      REQUIRE_THROWS_AS(game.open("/bad/game/path"), std::runtime_error);

      THEN("filePath() is empty") {
        REQUIRE(QString(game.filePath()).isEmpty());
      }

      THEN("copyToRam() throws a runtime error") {
        REQUIRE_THROWS_AS(game.copyToRam(), std::runtime_error);
        REQUIRE(game.size() == 0);
        REQUIRE(QByteArray(game.constData()) == QByteArray(""));
      }

    }

    QFile::remove(workingGamePath);
  }
}
