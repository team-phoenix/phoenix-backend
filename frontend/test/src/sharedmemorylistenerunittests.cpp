#include "catch.hpp"
#include "sharedmemorylistener.h"

SCENARIO("SharedMemoryListener")
{
  GIVEN("a mocked out video frame in shared memory") {

    static bool updateFlag = true;
    static uint width = 4;
    static uint height = 3;
    static char padding = 0xAA;
    static uint pitch = (sizeof(padding) * 2) + width;

    static const int inputBlockOffset = 16;
    static const int videoBlockSize = (pitch * height) * sizeof(char);
    static const int fakeVideoSize =        inputBlockOffset
                                            + sizeof(updateFlag)
                                            + sizeof(width)
                                            + sizeof(height)
                                            + sizeof(pitch)
                                            + videoBlockSize;

    static char* fakeVideoData = new char[fakeVideoSize];
    memcpy(fakeVideoData + inputBlockOffset, &updateFlag, sizeof(updateFlag));
    memcpy(fakeVideoData
           + inputBlockOffset
           + sizeof(updateFlag), reinterpret_cast<char*>(&width), sizeof(width));
    memcpy(fakeVideoData
           + inputBlockOffset
           + sizeof(updateFlag)
           + sizeof(width), reinterpret_cast<char*>(&height),
           sizeof(height));
    memcpy(fakeVideoData
           + inputBlockOffset
           + sizeof(updateFlag)
           + sizeof(width)
           + sizeof(height),
           reinterpret_cast<char*>(&pitch), sizeof(pitch));

    const int fakeVideoDataOffset = inputBlockOffset
                                    + sizeof(updateFlag)
                                    + sizeof(width)
                                    + sizeof(height)
                                    + sizeof(pitch);

    fakeVideoData[ fakeVideoDataOffset + 0 ] = 0x00;
    fakeVideoData[ fakeVideoDataOffset + 1 ] = 0x01;
    fakeVideoData[ fakeVideoDataOffset + 2 ] = 0x01;
    fakeVideoData[ fakeVideoDataOffset + 3 ] = 0x00;
    fakeVideoData[ fakeVideoDataOffset + 4 ] = padding;
    fakeVideoData[ fakeVideoDataOffset + 5 ] = padding;

    fakeVideoData[ fakeVideoDataOffset + 6 ] = 0x00;
    fakeVideoData[ fakeVideoDataOffset + 7] = 0x01;
    fakeVideoData[ fakeVideoDataOffset + 8 ] = 0x01;
    fakeVideoData[ fakeVideoDataOffset + 9 ] = 0x00;
    fakeVideoData[ fakeVideoDataOffset + 10 ] = padding;
    fakeVideoData[ fakeVideoDataOffset + 11 ] = padding;

    fakeVideoData[ fakeVideoDataOffset + 12 ] = 0x00;
    fakeVideoData[ fakeVideoDataOffset + 13 ] = 0x01;
    fakeVideoData[ fakeVideoDataOffset + 14 ] = 0x01;
    fakeVideoData[ fakeVideoDataOffset + 15 ] = 0x00;
    fakeVideoData[ fakeVideoDataOffset + 16 ] = padding;
    fakeVideoData[ fakeVideoDataOffset + 17 ] = padding;

    const int fakeVideoBlockSize = fakeVideoSize - fakeVideoDataOffset;
    QVector<uchar> fakeVideoBlockVector(fakeVideoBlockSize);

    for (int i = 0; i < fakeVideoBlockVector.size(); ++i) {
      fakeVideoBlockVector[i] = fakeVideoData[ fakeVideoDataOffset + i ];
    }

    static bool isAttachedCalled = false;

    struct MockMemory {
      enum AccessMode {
        ReadWrite,
        WriteOnly,
      };
      void lock() {}
      void unlock() {}
      bool isAttached()
      {
        return isAttachedCalled;
      }

      bool attach(AccessMode) { return isAttachedCalled = true; }
      void detach() {}
      void setKey(QString) {}
      QString errorString() { return QString(); }

      void* data()
      {
        return fakeVideoData;
      }
    };

    SharedMemoryListener_T<MockMemory> subject;

    WHEN("readVideoFrame(), is called") {

      QVector<uchar> imageBuffer(fakeVideoBlockSize);
      QImage image;
      const QImage::Format anyFormat = QImage::Format_RGB32;
      subject.readVideoFrame(imageBuffer, image, anyFormat);

      THEN("The video frame could be read from the shared memory buffer correctly") {
        REQUIRE(imageBuffer == fakeVideoBlockVector);

        THEN("The first index at the video block offset has the updateFlag set to false") {
          bool droppedFrame = true;
          memcpy(reinterpret_cast<char*>(&droppedFrame), fakeVideoData + inputBlockOffset,
                 sizeof(droppedFrame));

          REQUIRE(droppedFrame == false);
        }
      }
    }
  }
}
