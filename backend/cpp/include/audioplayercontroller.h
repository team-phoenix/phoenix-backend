#pragma once

#include "audioplayer.h"
#include "circularchunkbuffer.h"

#include <QThread>
#include <QObject>

class AudioController : public QObject
{
  Q_OBJECT
public:
  explicit AudioController(QObject* parent = nullptr);

  void play();
  void stop();

  void setSampleRate(double sampleRate);
  void writeFramesFromShortArray(const qint16* data, size_t frames);

  bool isListening() const;

private:
  AudioPlayer audioPlayer;
  QThread audioThread;
  CircularChunkBuffer ringBuffer;
};
