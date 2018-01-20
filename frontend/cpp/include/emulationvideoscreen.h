#pragma once

#include "sharedmemorylistener.h"

#include <QQuickItem>
#include <QImage>

struct VideoInfo {
  double aspectRatio{ 1.0 };
  int height{0};
  int width{0};
  double frameRate{0.0};
  QImage::Format pixelFormat;
};

class EmulationVideoScreen : public QQuickItem
{
  Q_OBJECT
public:
  explicit EmulationVideoScreen(QQuickItem* parent = nullptr);

  QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData*) override;

private slots:
  void prepareVideoFrame(double aspectRatio, int height, int width, double frameRate,
                         int pixelFormat);

private:
  QImage currentVideoFrame;
  VideoInfo currentVideoInfo;
  SharedMemoryListener sharedMemoryListener;
  QVector<uchar> currentVideoFrameBuffer;
};
