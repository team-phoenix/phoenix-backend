#include "audioplayer.h"

#include <QDebug>
#include <QFile>
#include <QThread>

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent),
  ioTimer(this),
  chunkBuffer(2046 * 60, 0x0),
  circularChunkBuffer(chunkBuffer.size() * 10 * 2)
{
  audioOutput = new QAudioOutput(getWavAudioFormat(), this);

  connect(audioOutput, &QAudioOutput::stateChanged, this, &AudioPlayer::onAudioStateChanged);

//  connect(&audioSource, &AudioSource::fullChunkProduced, this, &AudioPlayer::onFullChunkProduced);

//  audioSource.setCircularBuffer(&circularChunkBuffer);

  ioTimer.setTimerType(Qt::PreciseTimer);

  connect(&ioTimer, &QTimer::timeout, this, &AudioPlayer::onPushModeTimeout);
}

void AudioPlayer::play()
{
  ioOutput = audioOutput->start();
  ioTimer.setInterval(8);

  onPushModeTimeout();
  ioTimer.start();
}

void AudioPlayer::onAudioStateChanged(QAudio::State state)
{
  qDebug() << "state:" << state << audioOutput->error();

  if (state == QAudio::IdleState) {
    if (audioOutput->error() == QAudio::UnderrunError) {

    }
  }
}

void AudioPlayer::onPushModeTimeout()
{

  int chunks = audioOutput->bytesFree() / audioOutput->periodSize();

  QByteArray buffer(chunks * audioOutput->periodSize(), 0);

  while (chunks) {

    const qint64 len = circularChunkBuffer.read(buffer.data(), audioOutput->periodSize());

    if (len)
    { ioOutput->write(buffer.data(), len); }

    if (len != audioOutput->periodSize())
    { break; }

    --chunks;
  }

}

QAudioFormat AudioPlayer::getWavAudioFormat()
{
  QAudioFormat format;
  format.setSampleRate(44100);
  format.setChannelCount(2);
  format.setSampleSize(16);
  format.setCodec("audio/pcm");
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setSampleType(QAudioFormat::UnSignedInt);

  QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

  if (!info.isFormatSupported(format)) {
    qFatal("Raw audio format not supported by backend, cannot play audio.");
  }

  return format;
}
