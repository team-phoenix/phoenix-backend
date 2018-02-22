#include "emulationvideoscreen.h"
#include "emulationlistener.h"

#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QQuickWindow>

#include <QTimer>
#include <QMutexLocker>

EmulationVideoScreen::EmulationVideoScreen(QQuickItem* parent)
  : QQuickItem(parent)
{
  setFlag(QQuickItem::ItemHasContents, true);

  connect(&EmulationListener::instance(), &EmulationListener::videoInfoChanged, this,
          &EmulationVideoScreen::prepareVideoFrame);

  connect(&EmulationListener::instance(), &EmulationListener::startReadingFrames, this,
  [this] {
    qDebug() << "start reading frames";

    QTimer* timer = new QTimer(this);
    timer->setInterval(0);

    connect(timer, &QTimer::timeout, this, [this] {
      sharedMemoryListener.fillVideoFrame(currentVideoFrame, currentVideoInfo.pixelFormat);

      if (currentVideoFrame.isNull())
      {
        qDebug() << "video texture is null";
//        Q_ASSERT(false);
      } else
      {
        update();
      }

    });

    timer->start();

  });

  connect(&EmulationListener::instance(), &EmulationListener::pauseReadingFrames, this,
  [this] {
    qDebug() << "pause reading frames, stop timers";
  });

}

QSGNode* EmulationVideoScreen::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{

  QMutexLocker locker(&videoFrameMutex);

  if (!window() || currentVideoFrame.isNull()) {
    return node;
  }

  QSGSimpleTextureNode* textureNode = static_cast<QSGSimpleTextureNode*>(node);

  if (!textureNode) {
    textureNode = new QSGSimpleTextureNode;
  }

  QSGTexture* sgTexture = window()->createTextureFromImage(currentVideoFrame.getQImage());

  QRectF rect = boundingRect();
  textureNode->setTexture(sgTexture);
  textureNode->setRect(rect);
  textureNode->setFiltering(QSGTexture::Nearest);
  textureNode->setOwnsTexture(true);

  return textureNode;
}

qreal EmulationVideoScreen::aspectRatio() const
{
  return currentVideoInfo.aspectRatio;
}

void EmulationVideoScreen::prepareVideoFrame(double aspectRatio, int height, int width,
                                             double frameRate, int pixelFormat)
{
  currentVideoInfo.aspectRatio = aspectRatio == 0 ? 1 : aspectRatio;
  currentVideoInfo.height = height;
  currentVideoInfo.width = width;
  currentVideoInfo.frameRate = frameRate;
  currentVideoInfo.pixelFormat = static_cast<QImage::Format>(pixelFormat);
}

