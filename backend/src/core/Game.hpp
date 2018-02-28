#pragma once

#include <QFile>

template<typename GameFile = QFile,
         typename GameBuffer = QByteArray,
         typename GameFilePath = QByteArray>
class Game_T
{
public:

  Game_T() = default;

  ~Game_T()
  {
    clear();
    close();
  }

  void open(QString gamePath)
  {
    gameFile.setFileName(gamePath);

    if (!gameFile.exists()) {
      throw std::runtime_error(gamePath.toStdString() + " does not exist!");
    }

    gameFilePath = gamePath.toLocal8Bit();

    if (!gameFile.open(QIODevice::ReadOnly)) {
      throw std::runtime_error("could not open " + gamePath.toStdString());
    }
  }

  const char* constData() const
  {
    return buffer.constData();
  }

  int size() const
  {
    return buffer.size();
  }

  void close()
  {
    gameFile.close();
  }

  void clear()
  {
    gameFilePath.clear();
    buffer.clear();
  }

  const char* filePath() const
  {
    return gameFilePath.constData();
  }

  void copyToRam()
  {
    if (!buffer.isEmpty()) {
      throw std::runtime_error("the game buffer is not empty, it should be empty...");
    }

    buffer = gameFile.readAll();

    if (buffer.size() == 0) {
      throw std::runtime_error("The game buffer is empty after copying from the game");
    }
  }

private:
  GameBuffer buffer;
  GameFile gameFile;
  GameFilePath gameFilePath;
};

using Game = Game_T<>;
