#include "doctest.hpp"
#include "AudioResampler.h"
#include "samplerate.h"

#include <stdexcept>

SCENARIO("AudioResampler")
{
  GIVEN("a subject") {
    AudioResampler subject;

    REQUIRE(subject.getSrcState() == nullptr);

    WHEN("init() is called with a valid channel count") {
      const int channelCount = 2;
      subject.init(channelCount);

      REQUIRE(subject.getSrcState() != nullptr);
    }

    WHEN("init() is called with an invalid channel count it should throw") {
      const int channelCount = 999999;
      REQUIRE_THROWS_AS(subject.init(channelCount), std::runtime_error);
    }

  }
}
