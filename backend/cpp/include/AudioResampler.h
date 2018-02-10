#pragma once

#include <samplerate.h>

class AudioResampler
{
public:
  AudioResampler() = default;
  ~AudioResampler();

  void init(int channels);

  void resample(SRC_DATA* srcData);

public:

  SRC_STATE* getSrcState() const ;

private:
  SRC_STATE* srcState{ nullptr };

private:
  void checkForError(int error);

};
