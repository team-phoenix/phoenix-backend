#pragma once

#include "sharedmemorylistener.h"

#include <QQuickItem>
#include <QImage>
#include <QMutex>

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
  Q_PROPERTY(qreal aspectRatio READ aspectRatio NOTIFY aspectRatioChanged)

public:
  explicit EmulationVideoScreen(QQuickItem* parent = nullptr);

  QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData*) override;

  qreal aspectRatio() const;

private slots:
  void prepareVideoFrame(double aspectRatio, int height, int width, double frameRate,
                         int pixelFormat);

signals:
  void aspectRatioChanged();

private:
  VideoFrame currentVideoFrame;
  VideoInfo currentVideoInfo;
  SharedMemoryListener sharedMemoryListener;
  QMutex videoFrameMutex;
};
