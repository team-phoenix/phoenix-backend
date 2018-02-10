#pragma once

#include "circularchunkbuffer.h"
#include "AudioResampler.h"

#include <QObject>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>

class AudioPlayer : public QObject
{
  Q_OBJECT
public:
  explicit AudioPlayer(QObject* parent = nullptr);

  void setRingBuffer(CircularChunkBuffer* ringBuffer);
signals:

public slots:
  void play();
  void stop();
  void init(double sampleRate);

private slots:
  void onAudioStateChanged(QAudio::State state);
  void onPushModeTimeout();

  // Test case getters
public:
  const AudioResampler &getAudioResampler() const;

private:
  QAudioOutput* audioOutput{nullptr};
  QIODevice* ioOutput{nullptr};

  QTimer ioTimer;
  QByteArray currentChunk;

  CircularChunkBuffer* circularChunkBuffer;
  QAudioFormat audioFormat;
  AudioResampler audioResampler;

private:
  QAudioFormat getWavAudioFormat();
  void printUsefulTimingStatements();
//  AudioSource audioSource;
};
