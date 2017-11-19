#pragma once

#include <QFile>

class Game
{
public:

  Game() = default;

  ~Game()
  {
    clear();
    close();
  }

  void open(QString gamePath)
  {
    if (!QFile::exists(gamePath)) {
      throw std::runtime_error(gamePath.toStdString() + " does not exist!");
    }

    gameFile.setFileName(gamePath);
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
  }

private:
  QByteArray buffer;
  QFile gameFile;
  QByteArray gameFilePath;
};
