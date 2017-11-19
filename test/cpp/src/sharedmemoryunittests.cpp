#include "catch.hpp"

#include <QtGlobal>
#include <QVector>

#include "sharedmemorybuffer.h"

template<typename T, typename R>
T read(R* src, size_t offset)
{
  const size_t len = sizeof(T) / sizeof(char);
  char array[ len ];

  for (size_t i = 0; i < len; ++i) {
    array[ i ] = static_cast<char*>(src)[ offset ];
    offset += 1;
  }

  return *reinterpret_cast<T*>(array);
}

SCENARIO("Video data can be written to and read from mocked shared memory", "[SharedMemoryBuffer]")
{
  GIVEN("a buffer with some data") {
    const int bufferSize = 100;

    struct MockMemory {
      MockMemory() { buffer = QVector<char>(bufferSize, 0x0); }

      void* data()
      {
        return buffer.data();
      }

      int size() const
      {
        return buffer.size();
      }

      bool isAttached() const
      {
        return true;
      }

      bool detach() const
      {
        return true;
      }

      bool lock() { return true; }
      bool unlock() { return true; }

      void clear()
      {
        for (int i = 0; i < buffer.size(); ++i) {
          buffer[i] = '\0';
        }
      }

      QVector<char> buffer;
    };

    SharedMemoryBuffer_T<MockMemory> subject;

    const uint width = 4;
    const uint height = 3;
    const char padding = 0xAA;
    const uint pitch = (sizeof(padding) * 2) + width;
    const int fakeVideoSize = pitch * height;

    char* fakeVideoData = new char[fakeVideoSize];
    fakeVideoData[ 0 ] = 0x00;
    fakeVideoData[ 1 ] = 0x01;
    fakeVideoData[ 2 ] = 0x01;
    fakeVideoData[ 3 ] = 0x00;
    fakeVideoData[ 4 ] = padding;
    fakeVideoData[ 5 ] = padding;

    fakeVideoData[ 6 ] = 0x00;
    fakeVideoData[ 7 ] = 0x01;
    fakeVideoData[ 8 ] = 0x01;
    fakeVideoData[ 9 ] = 0x00;
    fakeVideoData[ 10 ] = padding;
    fakeVideoData[ 11 ] = padding;

    fakeVideoData[ 12 ] = 0x00;
    fakeVideoData[ 13 ] = 0x01;
    fakeVideoData[ 14 ] = 0x01;
    fakeVideoData[ 15 ] = 0x00;
    fakeVideoData[ 16 ] = padding;
    fakeVideoData[ 17 ] = padding;

    WHEN("writeVideoFrame() was called with a correct size") {

      const char* writtenBuffer = subject.writeVideoFrame(fakeVideoData, width, height, pitch);
      REQUIRE(writtenBuffer != nullptr);

      const int totalSize = sizeof(bool) + sizeof(width) + sizeof(height) + sizeof(pitch) + fakeVideoSize;
      QVector<char> writtenVector(totalSize);

      for (int i = 0; i < writtenVector.size(); ++i) {
        writtenVector[i] = writtenBuffer[i];
      }

      THEN("An update flag was written at start of buffer") {
        bool flag = read<bool>(writtenVector.data(), 0);
        REQUIRE(flag == true);
      }

      THEN("The width was written second") {
        size_t offset = sizeof(bool);
        uint frameWidth = read<uint>(writtenVector.data(), offset);
        REQUIRE(frameWidth == width);
      }

      THEN("The height was written third") {
        size_t offset = sizeof(bool) + sizeof(uint);
        uint frameHeight = read<uint>(writtenVector.data(), offset);
        offset += sizeof(frameHeight);
        REQUIRE(frameHeight == height);
      }

      THEN("The pitch was written fourth") {
        size_t offset = sizeof(bool) + sizeof(uint) * 2;
        uint framePitch = read<uint>(writtenVector.data(), offset);
        REQUIRE(framePitch == pitch);
      }

      THEN("The video data was written lastly") {
        size_t offset = sizeof(bool) + sizeof(uint) * 3;

        for (size_t i = offset; i < static_cast<size_t>(writtenVector.size()); ++i) {
          REQUIRE(writtenVector[i] == fakeVideoData[i - offset]);
        }
      }

      WHEN("clear() is called") {
        subject.clear();

        THEN("the memory buffer is zeroed out") {
          for (int i = 0; i < writtenVector.size(); ++i) {
            writtenVector[i] = writtenBuffer[i];
          }

          for (const char &c : writtenVector) {
            REQUIRE(c == '\0');
          }
        }
      }

      WHEN("writeVideoFrame() is called with a size greater than the memory size") {
        REQUIRE_THROWS_AS(subject.writeVideoFrame(fakeVideoData, width, height, bufferSize + 1),
                          std::runtime_error);
      }
    }
  }
}

SCENARIO("Keyboard states can be read from a mocked memory buffer", "[SharedMemoryBuffer]")
{
  GIVEN("A valid memory region that contains keyboard states") {

    static const QVector<qint16> fakeStates({
      0, 1, 2, 3, 4,
      5, 6, 7, 8, 9,
    });

    struct MockMemory {
      MockMemory() {buffer = fakeStates;}
      enum AccessMode { ReadOnly = 0, ReadWrite = 1};

      void* data() {return buffer.data();}
      bool isAttached() const {return false;}
      bool detach() const {return true;}
      void lock() {}
      void unlock() {}
      void setKey(const QString &) {}
      bool create(int, AccessMode) { return true; }
      QString errorString() { return QString(); }
      int size() const { return buffer.size() * sizeof(qint16); }

      QVector<qint16> buffer;
    };

    qint16 bufferStates[fakeStates.size()] = { 0xFF };

    SharedMemoryBuffer_T<MockMemory> subject;
    REQUIRE_NOTHROW(subject.open(fakeStates.size()));

    WHEN("readKeyboardStates() is called") {

      subject.readKeyboardStates(bufferStates, fakeStates.size() * sizeof(qint16));

      THEN("The states match the given buffer") {
        for (int i = 0; i < fakeStates.size(); ++i) {
          REQUIRE(fakeStates[i] == bufferStates[i]);
        }
      }
    }

    REQUIRE(subject.close() == true);
  }
}

SCENARIO("The shared memory buffer can opened and closed", "[SharedMemoryBuffer]")
{
  GIVEN("a region of detached memory is used") {

    static bool attached = false;

    struct MockMemory {
      enum AccessMode { ReadOnly = 0, ReadWrite = 1};
      bool isAttached() { return attached; }
      bool detach() { return true; }
      bool create(int, AccessMode) { return true; }
      QString errorString() { return QString(); }
      void setKey(const QString &) {}
    };

    SharedMemoryBuffer_T<MockMemory> subject;
    const int bufferSize = 10;

    WHEN("open() is called with a valid buffer size") {
      attached = false;
      REQUIRE_NOTHROW(subject.open(bufferSize));
    }

    WHEN("open() is called with a region of that is not attached memory") {
      attached = false;
      REQUIRE_NOTHROW(subject.open(bufferSize));
    }

    WHEN("open() is called with a region of attached memory") {
      attached = true;
      REQUIRE_THROWS_AS(subject.open(bufferSize), std::runtime_error);
    }

    WHEN("close() is called when open() was called before") {
      attached = false;
      subject.open(bufferSize);
      const bool closed = subject.close();
      THEN("returns true") {
        REQUIRE(closed == true);
      }
    }

    WHEN("close() is called on unopened memory") {
      const bool closed = subject.close();
      THEN("returns true") {
        REQUIRE(closed == true);
      }
    }
  }
}
