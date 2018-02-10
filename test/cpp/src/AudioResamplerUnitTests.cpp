#include "doctest.hpp"
#include "AudioResampler.h"
#include "samplerate.h"

#include <stdexcept>

SCENARIO("AudioResampler")
{
  GIVEN("a subject") {
    AudioResampler audioResampler;

    REQUIRE(audioResampler.getSrcState() == nullptr);

    WHEN("init() is called with a valid channel count") {
      const int channelCount = 2;
      audioResampler.init(channelCount);

      REQUIRE(audioResampler.getSrcState() != nullptr);
    }

    WHEN("init() is called with an invalid channel count it should throw") {
      const int channelCount = 999999;
      REQUIRE_THROWS_AS(audioResampler.init(channelCount), std::runtime_error);
    }
  }
}
