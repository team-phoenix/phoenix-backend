#include "audioplayercontroller.h"

#include <QDebug>

AudioController::AudioController()
{
  audioPlayer.setRingBuffer(&ringBuffer);
}

void AudioController::play()
{
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

void AudioController::write(const char* data, size_t frames)
{
  ringBuffer.write(data, frames);
}

bool AudioController::isListening() const
{
  return audioThread.isRunning();
}

