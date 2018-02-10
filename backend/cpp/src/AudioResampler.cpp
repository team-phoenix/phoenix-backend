#include "AudioResampler.h"
#include <stdexcept>

AudioResampler::~AudioResampler()
{
  src_delete(srcState);
}

void AudioResampler::init(int channels)
{
  int error;
  srcState = src_new(SRC_SINC_FASTEST, channels, &error);
  checkForError(error);
}

void AudioResampler::resample(SRC_DATA* srcData)
{
  const int error = src_process(srcState, srcData);
  checkForError(error);
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
