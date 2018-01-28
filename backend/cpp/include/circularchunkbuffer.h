#pragma once

#include <QtGlobal>

#include <atomic>
#include <cstddef>

// Circular buffer holding audio data.
// Should only be accessed by one consumer and one producer thread.
class CircularChunkBuffer
{

  char* m_buffer;
  std::atomic<size_t> m_head; // newest item in queue
  std::atomic<size_t> m_tail; // oldest item in queue
  const size_t m_size;

  size_t next(size_t current);

  size_t size_impl(size_t head, size_t tail) const;

public:
  CircularChunkBuffer(size_t size = 4096 * 8);
  ~CircularChunkBuffer();

  size_t write(const char* data, size_t size);
  size_t write(const qint16* data, size_t size)
  {

    for (size_t i = 0; i < size; ++i) {
      char c = static_cast<char>(data[i]);
      write(&c, sizeof(char));
    }

    return size;
  }

  size_t read(char* object, size_t size);


  size_t size() const;

  size_t capacity() const;

  void clear();

};

