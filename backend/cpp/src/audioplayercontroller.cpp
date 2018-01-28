#include "audioplayercontroller.h"

AudioController::AudioController()
{
  audioPlayer.moveToThread(&audioThread);
  Q_ASSERT(audioPlayer.thread() == &audioThread);
  Q_ASSERT(audioPlayer.thread() != QThread::currentThread());

  audioThread.start(QThread::HighestPriority);
}

void AudioController::play()
{
  QMetaObject::invokeMethod(&audioPlayer, "play");
}

