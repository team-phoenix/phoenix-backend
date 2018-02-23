#pragma once
#include <QObject>
#include <QThread>

#include "SuperTimerRunnable.h"

class SuperTimer : public QObject
{
  Q_OBJECT
public:
  explicit SuperTimer(QObject* parent = nullptr);

  void setTargetFPS(qreal fps)
  {
    const qreal targetFps = (1.0 / fps) * 1000.0;
    QMetaObject::invokeMethod(&superTimerRunnable,
                              "setTargetMilisecondInterval",
                              Q_ARG(qreal, targetFps));
  }

  void start()
  {
    QMetaObject::invokeMethod(&superTimerRunnable, "start");
    timerIsActive = true;
  }

  void stop()
  {
    QMetaObject::invokeMethod(&superTimerRunnable, "stop");
    timerIsActive = false;
  }

  bool isActive() const
  {
    return timerIsActive;
  }

public slots:
  void increaseAudioTimeout()
  {
    QMetaObject::invokeMethod(&superTimerRunnable, "increaseAudioTimeout");
  }
  void decreaseAudioTimeout()
  {
    QMetaObject::invokeMethod(&superTimerRunnable, "decreaseAudioTimeout");
  }

signals:
  void timeout();

private:

  QThread timerThread;
  SuperTimerRunnable superTimerRunnable;
  bool timerIsActive{false};

};
