#pragma once

#include "libretro.h"

#include <QtGlobal>
#include <QSharedMemory>
#include <QDebug>

static const QString MEMORY_KEY = QStringLiteral("PHX_FUR_LYFE_BABY!");
static const int MEMORY_KEY_STATES_OFFSET = 0;
static const int MEMORY_VIDEO_FRAME_OFFSET = (RETRO_DEVICE_ID_JOYPAD_R3 + 1) * sizeof(qint16);

template<typename Memory = QSharedMemory>
class SharedMemoryBuffer_T
{
public:

  SharedMemoryBuffer_T()
  {
    if (memory.isAttached()) {
      memory.detach();
    }
  }

  const char* writeVideoFrame(const char* data, uint width, uint height, uint pitch)
  {
    const bool flag = true;

    const int bufferSize = (height * pitch) * sizeof(char);

    checkSize(bufferSize);

    memory.lock();

    char* dest = static_cast<char*>(memory.data()) + videoFrameMemoryOffset();

    memcpy(dest, &flag, sizeof(flag));
    size_t offset = sizeof(flag);

    memcpy(dest + offset, &width, sizeof(width));
    offset += sizeof(width);

    memcpy(dest + offset, &height, sizeof(height));
    offset += sizeof(height);

    memcpy(dest + offset, &pitch, sizeof(pitch));
    offset += sizeof(pitch);

    memcpy(dest + offset, data, bufferSize);

    memory.unlock();
    return dest;
  }

  void readKeyboardStates(qint16* dest, size_t size)
  {
    checkSize(static_cast<int>(size));

    if (static_cast<int>(size) >= MEMORY_VIDEO_FRAME_OFFSET) {
      throw std::runtime_error("the keyboard state size is bigger than the "
                               "video frame's memory offset, it HAS to be smaller");
    }

    memory.lock();

    qint16* src = static_cast<qint16*>(memory.data());
    memcpy(dest, src + keyboardStatesMemoryOffset(), size);

    memory.unlock();
  }

  void checkSize(int size)
  {
    if (size > memory.size()) {
      if (!memory.isAttached()) {
        throw std::runtime_error("The memory is not attached, was it ever opened?");
      }

      throw std::runtime_error("The bufferSize is greater than the memory size");
    }
  }

  bool isOpened() const
  {
    return opened;
  }

  void open(int size)
  {
    memory.setKey(MEMORY_KEY);

    if (memory.isAttached()) {
      throw std::runtime_error("memory is attached, cannot open");
    }

    if (!memory.create(size, Memory::AccessMode::ReadWrite)) {
      throw std::runtime_error(memory.errorString().toStdString());
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

  void clear()
  {
    memory.lock();
    char* buffer = static_cast<char*>(memory.data());
    const char src = '\0';

    for (int i = 0; i < memory.size(); ++i) {
      memcpy(buffer + i, &src, sizeof(src));
    }

    memory.unlock();
  }

private:
  Memory memory;
  bool opened{ false };
};

using SharedMemoryBuffer = SharedMemoryBuffer_T<>;
