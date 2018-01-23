#include "emulationvideoscreen.h"
#include "emulationlistener.h"

#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QQuickWindow>

#include <QTimer>

EmulationVideoScreen::EmulationVideoScreen(QQuickItem* parent)
  : QQuickItem(parent)
{
  setFlag(QQuickItem::ItemHasContents, true);

  currentVideoFrame = QImage(250, 250, QImage::Format_RGB32);
  currentVideoFrame.fill(Qt::yellow);

  connect(&EmulationListener::instance(), &EmulationListener::videoInfoChanged, this,
          &EmulationVideoScreen::prepareVideoFrame);

  connect(&EmulationListener::instance(), &EmulationListener::startReadingFrames, this,
  [this] {
    qDebug() << "start reading frames";

    QTimer* timer = new QTimer(this);
    timer->setInterval(16);

    connect(timer, &QTimer::timeout, this, [this] {
      qDebug() << "reading";
      sharedMemoryListener.readVideoFrame(currentVideoFrameBuffer, currentVideoFrame, currentVideoInfo.pixelFormat);

      if (currentVideoFrame.isNull())
      {
        qDebug() << "video texture is null";
        Q_ASSERT(false);
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

  if (!window() || currentVideoFrame.isNull()) {
    qDebug() << "currentVideoFrame is null" << currentVideoFrame.isNull();
    return node;
  }

  qDebug() << "should be valid";
  QSGSimpleTextureNode* textureNode = static_cast<QSGSimpleTextureNode*>(node);

  if (!textureNode) {
    textureNode = new QSGSimpleTextureNode;
  }

  QSGTexture* sgTexture = window()->createTextureFromImage(currentVideoFrame);

  QRectF rect = boundingRect();

  if (currentVideoInfo.aspectRatio != 1.0) {
    rect.setWidth(rect.height() * currentVideoInfo.aspectRatio);
  }

  textureNode->setTexture(sgTexture);
  textureNode->setRect(rect);
  textureNode->setFiltering(QSGTexture::Nearest);

  return textureNode;
}

void EmulationVideoScreen::prepareVideoFrame(double aspectRatio, int height, int width,
                                             double frameRate, int pixelFormat)
{
  currentVideoInfo.aspectRatio = aspectRatio;
  currentVideoInfo.height = height;
  currentVideoInfo.width = width;
  currentVideoInfo.frameRate = frameRate;
  currentVideoInfo.pixelFormat = static_cast<QImage::Format>(pixelFormat);

}

