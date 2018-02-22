#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QCoreApplication>

#include <QDebug>
#include <QThread>

static const qreal INTERVAL_STEP_SIZE = 0.1;

class SuperTimerRunnable : public QObject
{
  Q_OBJECT
public:
  explicit SuperTimerRunnable(QObject* parent = nullptr)
    : QObject(parent)
  {

  }

public slots:

  void setTargetMilisecondInterval(qreal interval)
  {
    this->targetMilisecondInterval = interval;
  }

  void start()
  {
    Q_ASSERT(targetMilisecondInterval != 0.0);
    isRunning = true;
    loop();
  }

  void stop()
  {
    isRunning = false;
  }

  void loop()
  {
    quint64 totalNSecs = 0;
    const qreal closeEnoughDeviation = 0.2;

    while (isRunning) {

      if (!elapsedTimer.isValid()) {
        elapsedTimer.start();
      } else {
        const qint64 nsecsElapsed = elapsedTimer.nsecsElapsed();
        totalNSecs += nsecsElapsed;
        const auto milisecs = totalNSecs / 1000000.0;

        if (milisecs >= targetMilisecondInterval + 0.1
            || milisecs > targetMilisecondInterval - closeEnoughDeviation) {
          emit timeout();
          totalNSecs = 0;
        }

        elapsedTimer.start();
      }

      QCoreApplication::processEvents();

      QThread::yieldCurrentThread();
    }
  }

  void increaseAudioTimeout()
  {
    targetMilisecondInterval += INTERVAL_STEP_SIZE;
    qDebug() << "increasing audio timeout" << targetMilisecondInterval;
  }

  void decreaseAudioTimeout()
  {
    if (targetMilisecondInterval > 0) {
      targetMilisecondInterval -= INTERVAL_STEP_SIZE;
      qDebug() << "decreasing audio timeout" << targetMilisecondInterval;
    }
  }

signals:
  void timeout();

private:
  QElapsedTimer elapsedTimer;
  bool isRunning{ false };
  qreal targetMilisecondInterval{ 0.0 };
};
