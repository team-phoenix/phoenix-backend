#pragma once

#include <QImage>
#include <QVector>
class VideoFrame
{
public:
  VideoFrame();

  const QVector<uchar> &getFrameBuffer() const;
  const QImage &getQImage();

  void fillBuffer(const uchar* rawVideoData, int dataSize, unsigned width, unsigned height,
                  unsigned pitch, QImage::Format pixelFormat);

  bool isNull() const;
private:
  QVector<uchar> frameBuffer;
  QImage frameImage;
};
