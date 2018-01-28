#pragma once

#include "circularchunkbuffer.h"

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

signals:

public slots:
  void play();

private slots:
  void onAudioStateChanged(QAudio::State state);
  void onPushModeTimeout();

private:
  QAudioOutput* audioOutput{nullptr};
  QIODevice* ioOutput{nullptr};

  QTimer ioTimer;

  QByteArray chunkBuffer;
  int chunkBufferIndex{0};

  CircularChunkBuffer circularChunkBuffer;

private:
  QAudioFormat getWavAudioFormat();
//  AudioSource audioSource;
};
