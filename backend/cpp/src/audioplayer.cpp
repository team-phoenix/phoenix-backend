#include "audioplayer.h"
#include "logging.h"

#include <QDebug>
#include <QFile>
#include <QThread>

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent),
  ioTimer(this)
{
  ioTimer.setTimerType(Qt::PreciseTimer);
  connect(&ioTimer, &QTimer::timeout, this, &AudioPlayer::onPushModeTimeout);
}

void AudioPlayer::setRingBuffer(CircularChunkBuffer* ringBuffer)
{
  circularChunkBuffer = ringBuffer;
}

void AudioPlayer::play()
{
  ioOutput = audioOutput->start();
  currentChunk = QByteArray(audioOutput->bufferSize(), 0x0);

  ioTimer.setInterval(0);
  ioTimer.start();
}

void AudioPlayer::stop()
{
  ioTimer.stop();
}

void AudioPlayer::init(double sampleRate)
{
  audioFormat.setSampleRate(sampleRate);
  audioFormat.setChannelCount(2);
  audioFormat.setSampleSize(16);
  audioFormat.setCodec("audio/pcm");
  audioFormat.setByteOrder(QAudioFormat::LittleEndian);
  audioFormat.setSampleType(QAudioFormat::SignedInt);

  audioResampler.init(audioFormat.channelCount());

  QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

  if (!info.isFormatSupported(audioFormat)) {
    qFatal("Raw audio format not supported by backend, cannot play audio.");
  }

  audioOutput = new QAudioOutput(audioFormat, this);

  connect(audioOutput, &QAudioOutput::stateChanged, this, &AudioPlayer::onAudioStateChanged);
}

void AudioPlayer::onAudioStateChanged(QAudio::State state)
{
  qCDebug(phxAudioOutput) << state << audioOutput->error();

  if (state == QAudio::IdleState) {
    if (audioOutput->error() == QAudio::UnderrunError) {

    }
  }
}

void AudioPlayer::onPushModeTimeout()
{

  const int periodSize = audioOutput->periodSize();
  const int bytesFree = audioOutput->bytesFree();

  int chunks = bytesFree / periodSize;

  // Call printUsefulTimingStatements here for debug info
//  printUsefulTimingStatements();

  while (chunks) {

    const qint64 len = circularChunkBuffer->read(currentChunk.data(), periodSize);

    if (len) {
      ioOutput->write(currentChunk.data(), len);
    }

    if (len != periodSize) {
      break;
    }

    --chunks;
  }

}

const AudioResampler &AudioPlayer::getAudioResampler() const
{
  return this->audioResampler;
}

void AudioPlayer::printUsefulTimingStatements()
{
  const qreal chunkBufferRemainingRatio =  circularChunkBuffer->size() / static_cast<qreal>
                                           (circularChunkBuffer->capacity());
  qDebug() << chunkBufferRemainingRatio * 100.0 << "%";
}
