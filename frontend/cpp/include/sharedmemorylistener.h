#pragma once

#include <QObject>
#include <QSharedMemory>

static const QString MEMORY_KEY = QStringLiteral("PHX_FUR_LYFE_BABY!");
static const int MEMORY_KEY_STATES_OFFSET = 0;
static const int MEMORY_VIDEO_FRAME_OFFSET = (15 + 1) * sizeof(qint16);

template<typename T, typename R>
static T read(R* src, size_t &offset)
{
  const size_t len = sizeof(T) / sizeof(char);
  char array[ len ];

  for (size_t i = 0; i < len; ++i) {
    array[ i ] = static_cast<char*>(src)[ offset ];
    offset += 1;
  }

  return *reinterpret_cast<T*>(array);
}

template<typename Memory = QSharedMemory>
class SharedMemoryListener_T
{
public:

  SharedMemoryListener_T()
  {
    if (memory.isAttached()) {
      memory.detach();
    }
  }

  void readVideoFrame(char* dest, size_t destSize)
  {

    memory.lock();

    size_t offset = 16;

    char* rawMemoryBuffer = static_cast<char*>(memory.data());

    const bool updateFlag = read<bool>(rawMemoryBuffer, offset);

    if (updateFlag) {

      unsigned vidWidth = read<unsigned>(rawMemoryBuffer, offset);

      unsigned vidHeight = read<unsigned>(rawMemoryBuffer, offset);

      unsigned vidPitch = read<unsigned>(rawMemoryBuffer, offset);

      const uchar* vidBytes = reinterpret_cast<uchar*>(rawMemoryBuffer) + offset;

      const int bufferSize = (vidHeight * vidPitch) * sizeof(char);

      Q_ASSERT(destSize == bufferSize);

      for (int i = 0; i < destSize; ++i) {
        dest[ i ] = vidBytes[ i ];
      }


      offset = 16;
      static const bool droppedFrame = false;
      memcpy(rawMemoryBuffer + offset, &droppedFrame, sizeof(droppedFrame));

    }

    memory.unlock();
  }

//  void readKeyboardStates(qint16* dest, size_t size)
//  {
//    checkSize(static_cast<int>(size));

//    if (static_cast<int>(size) >= MEMORY_VIDEO_FRAME_OFFSET) {
//      throw std::runtime_error("the keyboard state size is bigger than the "
//                               "video frame's memory offset, it HAS to be smaller");
//    }

//    memory.lock();

//    qint16* src = static_cast<qint16*>(memory.data());
//    memcpy(dest, src + keyboardStatesMemoryOffset(), size);

//    memory.unlock();
//  }

//  void checkSize(int size)
//  {
//    if (size > memory.size()) {
//      if (!memory.isAttached()) {
//        throw std::runtime_error("The memory is not attached, was it ever opened?");
//      }

//      throw std::runtime_error("The bufferSize is greater than the memory size");
//    }
//  }

  bool isOpened() const
  {
    return opened;
  }

  void open()
  {
    memory.setKey(MEMORY_KEY);

    if (memory.isAttached()) {
      throw std::runtime_error("memory is attached, cannot open");
    }

    opened = true;
  }

  int keyboardStatesMemoryOffset() const
  {
    return MEMORY_KEY_STATES_OFFSET;
  }

  int videoFrameMemoryOffset() const
  {
    return MEMORY_VIDEO_FRAME_OFFSET;
  }

  bool close()
  {
    return (opened = memory.detach());
  }

  int size() const
  {
    return memory.size();
  }

  bool attach()
  {
    return memory.attach(Memory::AccessMode::ReadWrite);
  }

private:
  Memory memory;
  bool opened{ false };
};

using SharedMemoryListener = SharedMemoryListener_T<>;
