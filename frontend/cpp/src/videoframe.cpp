#include "videoframe.h"

VideoFrame::VideoFrame()
{
  frameImage = QImage(1, 1, QImage::Format_RGB32);
  frameImage.fill(Qt::black);
}

const QVector<uchar> &VideoFrame::getFrameBuffer() const
{
  return frameBuffer;
}

const QImage &VideoFrame::getQImage()
{
  return frameImage;
}

bool VideoFrame::isNull() const
{
  return frameImage.isNull();
}

void VideoFrame::fillBuffer(const uchar* rawVideoData, int dataSize, unsigned width,
                            unsigned height, unsigned pitch, QImage::Format pixelFormat)
{
  if (frameBuffer.size() < dataSize) {
    frameBuffer.resize(dataSize);
  }

  Q_ASSERT(frameBuffer.size() == dataSize);

  for (int i = 0; i < dataSize; ++i) {
    frameBuffer[ i ] = rawVideoData[ i ];
  }

  frameImage = QImage(frameBuffer.data()
                      , width, height, pitch
                      , pixelFormat);
}

