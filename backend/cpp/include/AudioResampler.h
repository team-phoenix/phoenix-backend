#pragma once

#include <samplerate.h>
#include <QVector>
#include "circularchunkbuffer.h"

#include <QAudioFormat>

static const int DEFAULT_RESAMPLE_RATE = 44100;

class AudioResampler
{
public:
  AudioResampler();
  ~AudioResampler();

  void init(const QAudioFormat &originalAudioFormat, size_t bufferSize,
            int sampleRate = DEFAULT_RESAMPLE_RATE);

  int resample(CircularChunkBuffer &circularChunkBuffer, QByteArray &dest, size_t bytesToConvert);
  QAudioFormat getOutputAudioFormat();



public:
  SRC_STATE* getSrcState() const;

private:
  SRC_STATE* srcState{ nullptr };

  QByteArray tempChunkBuffer;

  QVector<float> inputDataFloat;
  QVector<float> outputDataFloat;

  QAudioFormat outputAudioFormat;
  QAudioFormat originalAudioFormat;

public:
  void checkForError(int error);
  void setBufferSizes(size_t size);
  // WORKS!!!
  static int charsToFloat(const char* chars, float* floats, size_t bytesToConvert);
};
