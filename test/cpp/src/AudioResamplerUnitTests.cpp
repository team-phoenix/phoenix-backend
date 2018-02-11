#include "doctest.hpp"
#include "AudioResampler.h"
#include "samplerate.h"

#include <stdexcept>

SCENARIO("I want to know how to use libsamplerate properly")
{
  GIVEN("some input samples") {
    const QVector<uchar> charInput = {
      0xcd, 0xcc, 0xcc, 0x3d, // 0.1
      0x00, 0x00, 0x00, 0xbf, // -0.5
      0xcd, 0xcc, 0x4c, 0x3e, // 0.2
      0x9a, 0x99, 0x99, 0xbe, // -0.3
    };

    const QVector<float> input = {0.1, -0.5, 0.2, -0.3};
    const QVector<float> expected = {0.1, -0.5, 0.1, -0.5, 0.1, -0.5, 0.15, -0.4};
    QVector<float> output(expected.size());

    const int channels = 2;

    int err;
    SRC_STATE* srcState = src_new(SRC_LINEAR, channels, &err);
    REQUIRE(err <= 0);

    SRC_DATA srcData;

    WHEN("src_process() is called on a float buffer") {

      srcData.data_in = input.data();
      srcData.data_out = output.data();
      srcData.src_ratio = 2.0;
      srcData.end_of_input = 0;
      srcData.input_frames = input.size() / channels;
      srcData.output_frames = 8;

      int error = src_process(srcState, &srcData);
      REQUIRE(error <= 0);

      THEN("the output frames are resampled properly") {
        REQUIRE(expected == output);
      }

    }

    WHEN("charsToFloat() is called on a char buffer") {
      QVector<float> floatInput = { 0.0, 0.0, 0.0, 0.0 };
      int floatsCreated = AudioResampler::charsToFloat(reinterpret_cast<const char*>(charInput.data()),
                                                       floatInput.data(),
                                                       charInput.size());

      THEN("the chars were converted perfectly into a float array") {
        REQUIRE(floatsCreated == 4);
        REQUIRE(floatInput == input);
      }

    }

    src_delete(srcState);
  }
}


SCENARIO("AudioResampler")
{
  GIVEN("a subject") {
    AudioResampler subject;
    const int bufferSize = 10;

    REQUIRE(subject.getSrcState() == nullptr);

    WHEN("init() is called with a valid channel count") {

      QAudioFormat format;
      format.setChannelCount(2);

      subject.init(format, bufferSize);

      REQUIRE(subject.getSrcState() != nullptr);
    }

    WHEN("init() is called with an invalid channel count it should throw") {

      QAudioFormat format;
      format.setChannelCount(999999);

//      REQUIRE_THROWS_AS(subject.init(format, bufferSize), std::runtime_error);
    }


    WHEN("charsToFloat() is called on a buffer of chars") {
      const uchar buffer[4] = { 0xcd, 0xcc, 0xcc, 0x3d};
      float floatBuffer = 0.0;
      int floatsCreated = subject.charsToFloat(reinterpret_cast<const char*>(buffer), &floatBuffer, 4);

      THEN("the char buffer can be converted into a float buffer") {
        REQUIRE(floatsCreated == 1);
        REQUIRE(floatBuffer == 0.1f);
      }
    }

    WHEN("resample() is called") {

      const QVector<uchar> charInput = {
        0xcd, 0xcc, 0xcc, 0x3d, // 0.1
        0x00, 0x00, 0x00, 0xbf, // -0.5
        0xcd, 0xcc, 0x4c, 0x3e, // 0.2
        0x9a, 0x99, 0x99, 0xbe, // -0.3
      };


      QVector<uchar> expectedConvertedOutput(charInput.size() * 2);
      const QVector<float> expectedFloatOutput = {
        0.1,
        -0.5,
        0.1,
        -0.5,
        0.1,
        -0.5,
        0.15,
        -0.4
      };
      memcpy(expectedConvertedOutput.data(), expectedFloatOutput.data(), expectedConvertedOutput.size());

      CircularChunkBuffer circularChunkBuffer(charInput.size());
      int wrote = circularChunkBuffer.write(reinterpret_cast<const char*>(charInput.data()),
                                            charInput.size());

      REQUIRE(wrote == charInput.size());

      QAudioFormat audioFormat;
      audioFormat.setChannelCount(2);
      audioFormat.setSampleRate(48000);
      audioFormat.setSampleSize(16);
      audioFormat.setCodec("audio/pcm");
      audioFormat.setByteOrder(QAudioFormat::LittleEndian);
      audioFormat.setSampleType(QAudioFormat::SignedInt);
      \
      const int outputSampleRate = audioFormat.sampleRate() / 2;

      subject.init(audioFormat, expectedConvertedOutput.size(), outputSampleRate);

      QByteArray cachedChunk(circularChunkBuffer.size() * 2, '\0');

      int bytesResamples = subject.resample(circularChunkBuffer, cachedChunk, circularChunkBuffer.size());

      bool b = false;

      QVector<uchar> convertedChunkAsVector(cachedChunk.size(), '\0');

      for (int i = 0; i < cachedChunk.size(); ++i) {
        convertedChunkAsVector[i] = cachedChunk[i];
      }

      REQUIRE(bytesResamples == charInput.size());
      REQUIRE(expectedConvertedOutput == convertedChunkAsVector);
    }

  }
}
