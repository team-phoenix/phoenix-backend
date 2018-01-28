#pragma once

#include "audioplayer.h"
#include "circularchunkbuffer.h"

#include <QThread>

class AudioController
{
public:
  AudioController();

  void play();
  void stop();

  void setSampleRate(double sampleRate);
  void write(const char* data, size_t frames);

  bool isListening() const;

private:
  AudioPlayer audioPlayer;
  QThread audioThread;
  CircularChunkBuffer ringBuffer;
};
