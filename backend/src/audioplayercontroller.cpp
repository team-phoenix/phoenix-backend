#include "audioplayercontroller.h"

#include <QDebug>

AudioController::AudioController(QObject* parent)
  : QObject(parent)
{
  audioPlayer.setRingBuffer(&ringBuffer);
}

void AudioController::play()
{
  ringBuffer.clear();
  audioPlayer.moveToThread(&audioThread);
  Q_ASSERT(audioPlayer.thread() == &audioThread);
  Q_ASSERT(audioPlayer.thread() != QThread::currentThread());

  audioThread.start(QThread::HighestPriority);
  QMetaObject::invokeMethod(&audioPlayer, "play");
}

void AudioController::stop()
{
  QMetaObject::invokeMethod(&audioPlayer, "stop");
}

void AudioController::setSampleRate(double sampleRate)
{
  audioPlayer.init(sampleRate);
}

void AudioController::writeFramesFromShortArray(const qint16* data, size_t frames)
{
  ringBuffer.writeFramesFromShortArray(data, frames);
}

bool AudioController::isListening() const
{
  return audioThread.isRunning();
}

