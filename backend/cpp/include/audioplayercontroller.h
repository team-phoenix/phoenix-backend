#pragma once

#include "audioplayer.h"

#include <QThread>

class AudioController
{
public:
  AudioController();

  void play();

private:
  AudioPlayer audioPlayer;
  QThread audioThread;
};
