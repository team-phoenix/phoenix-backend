#include "AudioResampler.h"
#include <stdexcept>

#include <QDebug>


AudioResampler::AudioResampler()
{

}

AudioResampler::~AudioResampler()
{
  src_delete(srcState);
}

void AudioResampler::init(const QAudioFormat &originalAudioFormat, size_t bufferSize,
                          int sampleRate)
{
  setBufferSizes(bufferSize);

  this->originalAudioFormat = originalAudioFormat;
  outputAudioFormat = originalAudioFormat;
  outputAudioFormat.setSampleRate(sampleRate);

  int error;
  srcState = src_new(SRC_LINEAR, originalAudioFormat.channelCount(), &error);
  checkForError(error);
}

int AudioResampler::resample(CircularChunkBuffer &circularChunkBuffer, QByteArray &dest,
                             size_t bytesToConvert)
{

  const int samplesPerFrame = outputAudioFormat.channelCount();
  const int shortReadSize = tempChunkBuffer.size() / sizeof(qint16);

  int inputBytes = bytesToConvert;
  int inputFrames = originalAudioFormat.framesForBytes(bytesToConvert);
  int inputSamples = inputFrames * samplesPerFrame;

  int outputLengthMs = static_cast<int>(outputAudioFormat.durationForBytes(bytesToConvert)) / 1000;
  int outputTotalBytes = outputAudioFormat.bytesForDuration(outputLengthMs * 1000);
  int outputCurrentByte = bytesToConvert;
  int outputFreeBytes = outputTotalBytes - outputCurrentByte;
  int outputFreeFrames = outputAudioFormat.framesForBytes(outputFreeBytes);
  int outputFreeSamples = outputFreeFrames * samplesPerFrame;

  const qreal sampleRateRatio = originalAudioFormat.sampleRate() / static_cast<qreal>
                                (outputAudioFormat.sampleRate());

  const qreal adjustedSampleRateRatio = sampleRateRatio;
//  Q_UNUSED(outputFreeSamples);


  Q_ASSERT(tempChunkBuffer.data());

  qDebug() << bytesToConvert << circularChunkBuffer.size() << circularChunkBuffer.capacity();
  const int read = circularChunkBuffer.read(tempChunkBuffer.data(), bytesToConvert);

  const int floatsConverted = charsToFloat(tempChunkBuffer.data(), inputDataFloat.data(), read);

  SRC_DATA srcData;
  srcData.data_in = inputDataFloat.data();
  srcData.data_out = outputDataFloat.data();
  srcData.end_of_input = 0;
  srcData.input_frames = floatsConverted / 2;
  srcData.output_frames = adjustedSampleRateRatio * floatsConverted;
  srcData.src_ratio = adjustedSampleRateRatio;

//  src_set_ratio(srcState, srcData.src_ratio);
  const int error = src_process(srcState, &srcData);
  checkForError(error);

  const int outputFramesConverted = static_cast<int>(srcData.output_frames_gen);
  const int outputBytesConverted = outputAudioFormat.bytesForFrames(outputFramesConverted);
  const int outputSamplesConverted = outputFramesConverted * samplesPerFrame;

//  src_float_to_short_array(outputDataFloat.data(), outputDataShort.data(), outputSamplesConverted);
  char* outputDataChars = reinterpret_cast<char*>(outputDataFloat.data());

  for (int i = 0; i < outputBytesConverted * 2; ++i) {
    dest[i] = outputDataChars[i];
  }

  qDebug() << outputFramesConverted <<  outputBytesConverted << bytesToConvert <<
           circularChunkBuffer.size();

//  return 0;
  return outputBytesConverted;

}

QAudioFormat AudioResampler::getOutputAudioFormat()
{
  return outputAudioFormat;
}

SRC_STATE* AudioResampler::getSrcState() const
{
  return srcState;
}

void AudioResampler::checkForError(int error)
{
  if (error) {
    const char* errorString = src_strerror(error);
    throw std::runtime_error(errorString);
  }
}

void AudioResampler::setBufferSizes(size_t size)
{
  tempChunkBuffer = QByteArray(size, '\0');
  inputDataFloat.resize(tempChunkBuffer.size());
  outputDataFloat.resize(tempChunkBuffer.size());

  for (int i = 0; i < tempChunkBuffer.size(); ++i) {
    inputDataFloat[i] = 0.0;
    outputDataFloat[i] = 0.0;
  }

}

int AudioResampler::charsToFloat(const char* chars, float* floats, size_t bytesToConvert)
{
  int converted = 0;
  float* fp = (float*)chars;

  while (bytesToConvert >= sizeof * fp) {
    memcpy(floats++, fp++, sizeof * fp);
    bytesToConvert -= sizeof * fp;
    ++converted;
  }

  return converted;
}
