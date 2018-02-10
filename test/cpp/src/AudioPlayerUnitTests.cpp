#include "doctest.hpp"
#include "audioplayer.h"

SCENARIO("AudioPlayerUnitTests")
{
  GIVEN("a real subject") {

    AudioPlayer audioPlayer;

    const int validChannelCount = 2;

    WHEN("init() is called with a valid channel count") {
      const AudioResampler &audioResampler = audioPlayer.getAudioResampler();

      REQUIRE(audioResampler.getSrcState() == nullptr);
      REQUIRE_NOTHROW(audioPlayer.init(validChannelCount));

      THEN("the audio resampler gets initialized") {
        REQUIRE(audioResampler.getSrcState() != nullptr);
      }
    }
  }
}
