#include "catch.hpp"
#include "sharedmemorybuffer.h"

#include <limits>

#include <QVector>
#include <QSharedMemory>

SCENARIO("Video data can be written to and read from real shared memory", "[SharedMemoryBuffer]")
{
  GIVEN("a buffer with some data") {
    SharedMemoryBuffer subject;
    QVector<char> videoFrame({
      '0', '1', '2', '3', 'B', 'B',
      '4', '5', '6', '7', 'B', 'B'
    });
    const uint width = 4;
    const uint height = 2;
    const uint pitch = width + 2;

    REQUIRE_NOTHROW(subject.open(pitch));

    WHEN("writeVideoFrame() is called with a correct buffer size") {

      const char* writtenFrame = subject.writeVideoFrame(videoFrame.constData(), width, height, pitch);
      REQUIRE(writtenFrame != nullptr);

      const int totalSize = sizeof(bool) + sizeof(width) + sizeof(height) + sizeof(
                              pitch) + videoFrame.size();
      QVector<char> writtenVideoFrame(totalSize, 0x0);

      for (int i = 0; i < writtenVideoFrame.size(); ++i) {
        writtenVideoFrame[ i ] = writtenFrame[ i ];
      }

      THEN("The written video frame matches input frame") {
        const int offset = sizeof(bool) + (sizeof(uint) * 3);

        for (int i = offset; i < writtenVideoFrame.size(); ++i) {
          REQUIRE(writtenVideoFrame[i] == videoFrame[i - offset]);
        }
      }
    }

    WHEN("writeVideoFrame() is called with an larger than opened size") {
      REQUIRE_THROWS_AS(subject.writeVideoFrame(videoFrame.constData(), width, height,
                                                subject.size() + 1), std::runtime_error);
    }

    REQUIRE(subject.close() == true);
  }
}

SCENARIO("Keyboard states can be read from a real memory buffer", "[SharedMemoryBuffer]")
{
  GIVEN("A valid memory region that contains keyboard states") {

    SharedMemoryBuffer subject;

    QVector<qint16> fakeKeyStates({
      0x00, 0x01, 0xFF, 0xAA,
    });

    QSharedMemory mockedFrontendKeyStatesBuffer;
    mockedFrontendKeyStatesBuffer.setKey(MEMORY_KEY);

    REQUIRE(mockedFrontendKeyStatesBuffer.create(fakeKeyStates.size(),
                                                 QSharedMemory::AccessMode::ReadWrite) == true);

    REQUIRE(mockedFrontendKeyStatesBuffer.lock() == true);
    qint16* buffer = static_cast<qint16*>(mockedFrontendKeyStatesBuffer.data());
    memcpy(buffer, fakeKeyStates.constData(), fakeKeyStates.size() * sizeof(qint16));
    REQUIRE(mockedFrontendKeyStatesBuffer.unlock() == true);

    REQUIRE_THROWS_AS(subject.open(fakeKeyStates.size()), std::runtime_error);
    REQUIRE(subject.attach() == true);

    WHEN("readKeyboardStates() is called") {

      QVector<qint16> emptyKeyStates(fakeKeyStates.size(), 0x0);

      subject.readKeyboardStates(emptyKeyStates.data(), emptyKeyStates.size() * sizeof(qint16));

      THEN("The states match the given buffer") {
        for (int i = 0; i < emptyKeyStates.size(); ++i) {
          REQUIRE(emptyKeyStates[i] == fakeKeyStates[i]);
        }
      }
    }

    WHEN("readKeyboardStates() is called with a size >= the video frame offset") {
      REQUIRE_THROWS_AS(subject.readKeyboardStates(nullptr, MEMORY_VIDEO_FRAME_OFFSET),
                        std::runtime_error);
    }

    REQUIRE(subject.close() == true);
    REQUIRE(mockedFrontendKeyStatesBuffer.detach() == true);
  }
}
