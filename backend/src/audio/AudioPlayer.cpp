#include "audioplayer.h"
#include "Logging.h"

#include <QDebug>
#include <QFile>
#include <QThread>

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent),
  audioTimer(this)
{
  audioTimer.setTimerType(Qt::PreciseTimer);
  audioTimer.setInterval(0);

  currentChunk.fill('\0');
  connect(&audioTimer, &QTimer::timeout, this, &AudioPlayer::onPushModeTimeout);
}

void AudioPlayer::setRingBuffer(CircularChunkBuffer* ringBuffer)
{
  circularChunkBuffer = ringBuffer;
}

void AudioPlayer::play()
{
  ioOutput = audioOutput->start();
  audioTimer.start();

  qCDebug(phxAudioOutput) << "The audio output buffer size is" << audioOutput->bufferSize();
}

void AudioPlayer::stop()
{
  audioOutput->stop();
}

void AudioPlayer::init(double sampleRate)
{
  audioFormat.setSampleRate(sampleRate);
  audioFormat.setChannelCount(2);
  audioFormat.setSampleSize(16);
  audioFormat.setCodec("audio/pcm");
  audioFormat.setByteOrder(QAudioFormat::LittleEndian);
  audioFormat.setSampleType(QAudioFormat::SignedInt);

  const int bufferSize = 1024 * 60;

  QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

  const QAudioFormat outputAudioFormat = audioFormat; /*audioResampler.getOutputAudioFormat();*/

  if (!info.isFormatSupported(outputAudioFormat)) {
    qFatal("Raw audio format not supported by backend, cannot play audio.");
    throw std::runtime_error("Raw audio format not supported by backend, cannot play audio.");
  }

  audioOutput = new QAudioOutput(outputAudioFormat, this);

  connect(audioOutput, &QAudioOutput::stateChanged, this, &AudioPlayer::onAudioStateChanged);
}

void AudioPlayer::onAudioStateChanged(QAudio::State state)
{
  qCDebug(phxAudioOutput) << state << audioOutput->error();


//  if (state == QAudio::IdleState) {

//  } else if (state == QAudio::StoppedState) {
//    circularChunkBuffer->clear();
//    ioOutput = audioOutput->start();
//  }
}

void AudioPlayer::onPushModeTimeout()
{

  const size_t periodSize = audioOutput->periodSize();
  const int bytesFree = audioOutput->bytesFree();

  int chunks = bytesFree / periodSize;

  // Call printUsefulTimingStatements here for debug info

//  if (periodSize > circularChunkBuffer->size()) {
//    return;
//  }

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

void AudioPlayer::printUsefulTimingStatements()
{
  const qreal chunkBufferRemainingRatio =  circularChunkBuffer->size() / static_cast<qreal>
                                           (circularChunkBuffer->capacity());

  qDebug() << chunkBufferRemainingRatio * 100.0 << "%";
}
