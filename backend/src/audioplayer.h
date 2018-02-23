#pragma once

#include "circularchunkbuffer.h"

#include <QObject>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>

#include <array>

class AudioPlayer : public QObject
{
  Q_OBJECT
public:
  explicit AudioPlayer(QObject* parent = nullptr);

  void setRingBuffer(CircularChunkBuffer* ringBuffer);

signals:
  void audioBufferMayOverrun();
  void audioBufferWillUnderflow();

public slots:
  void play();
  void stop();
  void init(double sampleRate);

private slots:
  void onAudioStateChanged(QAudio::State state);
  void onPushModeTimeout();

private:
  QAudioOutput* audioOutput{nullptr};
  QIODevice* ioOutput{nullptr};

  std::array<char, 1024 * 30> currentChunk;

  CircularChunkBuffer* circularChunkBuffer;
  QAudioFormat audioFormat;

  bool trendingUpwards{ false };
  bool trendingDownwards{ false };
  bool passedHalfwayMark{ false };
  bool passedThirdQuarterMark{ false };
  bool passedFirstQuarterMark{ false };

  qint64 underrunCount{ 0 };
  QTimer audioTimer;

private:
  QAudioFormat getWavAudioFormat();
  void printUsefulTimingStatements();
//  AudioSource audioSource;
};
