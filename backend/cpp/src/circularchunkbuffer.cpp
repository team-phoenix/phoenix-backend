#include "circularchunkbuffer.h"

#include <QDebug>

CircularChunkBuffer::CircularChunkBuffer(size_t size)
  : m_head(0),
    m_tail(0),
    m_size(size + 1)
{
  m_buffer = new char[m_size];
}

CircularChunkBuffer::~CircularChunkBuffer()
{
  delete [] m_buffer;
}

size_t CircularChunkBuffer::write(const char* data, size_t size)
{
  size_t wrote = 0;
  size_t head, tail, nextHead;

  while (wrote < size) {
    head = m_head.load(std::memory_order_relaxed);
    nextHead = next(head);
    tail = m_tail.load(std::memory_order_acquire);

    if (nextHead == tail) {
      // buffer is full. let's just clear it.
      // It probably means the core produces frames too fast (not
      // clocked right) or audio backend stopped reading frames.
      // In the first case, it might cause audio to skip a bit.
      qDebug("Buffer full, dropping samples");
      clear();
    }


    m_buffer[head] = data[wrote++];
    m_head.store(nextHead, std::memory_order_release);
  }

  return wrote;
}

size_t CircularChunkBuffer::writeFramesFromShortArray(const qint16* src, size_t frames,
                                                      int channelCount)
{
  return write(reinterpret_cast<const char*>(src), frames * sizeof(qint16) * channelCount);
}

size_t CircularChunkBuffer::read(char* data, size_t size)
{
  size_t read = 0;
  size_t head, tail;

  while (read < size) {
    tail = m_tail.load(std::memory_order_relaxed);
    head = m_head.load(std::memory_order_acquire);

    if (tail == head) {
//      qDebug() << "overrun detected";
      break;
    }

    data[read++] = m_buffer[tail];
    m_tail.store(next(tail), std::memory_order_release);
  }

  return read;
}

size_t CircularChunkBuffer::next(size_t current)
{
  return (current + 1) % m_size;
}

size_t CircularChunkBuffer::size_impl(size_t tail, size_t head) const
{
  if (head < tail) {
    return head + (m_size - tail);
  } else {
    return head - tail;
  }
}

size_t CircularChunkBuffer::size() const
{
  size_t head = m_head.load(std::memory_order_relaxed);
  size_t tail = m_tail.load(std::memory_order_relaxed);
  return size_impl(tail, head);
}

size_t CircularChunkBuffer::capacity() const
{
  return m_size - 1;
}

void CircularChunkBuffer::clear()
{
  m_head.store(m_tail.load(std::memory_order_relaxed), std::memory_order_relaxed);
}
