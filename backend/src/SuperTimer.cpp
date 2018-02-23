#include "SuperTimer.h"

#include <QCoreApplication>
#include <QThread>
SuperTimer::SuperTimer(QObject* parent) : QObject(parent)
{
  connect(&superTimerRunnable, &SuperTimerRunnable::timeout, this, &SuperTimer::timeout);

  superTimerRunnable.moveToThread(&timerThread);
  timerThread.start(QThread::HighestPriority);

  Q_ASSERT(&timerThread == superTimerRunnable.thread());
  Q_ASSERT(this->thread() != &timerThread);
}
