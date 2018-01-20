#pragma once

#include "corecontroller.hpp"
#include "restServer.hpp"
#include "logging.h"

#include <QObject>
#include <QTimer>

class EmulationLoop : public QObject
{
  Q_OBJECT
public:


  enum LoopState {
    Uninitialized,
    Initialized,
    Playing,
    Paused,
  };


  EmulationLoop()
  {
    connect(&messageServer, &RestServer::requestRecieved, this, &EmulationLoop::parseRequest);
    connect(&messageServer, &RestServer::socketDisconnected, this, &EmulationLoop::stopTimer);
    connect(&emulationTimer, &QTimer::timeout, this, &EmulationLoop::loop);
  }

  void startTimerWithInterval(int miliseconds)
  {
    stopTimer();
    emulationTimer.setTimerType(Qt::PreciseTimer);
    emulationTimer.setInterval(miliseconds);
    startTimer();
  }

  ~EmulationLoop() = default;

private:
  LoopState looperState{ Uninitialized };

private slots:

  void stopTimer()
  {
    emulationTimer.stop();
    looperState = Paused;
    qCDebug(phxLoop) << "timer was stopped";
  }

  void startTimer()
  {
    looperState = Playing;
    emulationTimer.start();
  }

  void loop()
  {
    coreController.run();
  }

  void parseRequest(QJsonObject request)
  {
    const QString requestType = request[ "request" ].toString().simplified();
    qCDebug(phxLoop, "request %s", qPrintable(requestType));

    if (requestType == "initEmu") {
      if (looperState == Uninitialized) {
        const QString corePath = request[ "core" ].toString();
        const QString gamePath = request[ "game" ].toString();
        coreController.init(corePath, gamePath);
        looperState = Initialized;
        qCDebug(phxLoop) << "initialized emulation";
      } else {
        qCDebug(phxLoop) << "The emulator is already initialized, rejecting initialize request";
      }
    } else if (requestType == "playEmu") {

      if (looperState == Initialized) {
        const int miliseconds = 16;
        startTimerWithInterval(miliseconds);
        qCDebug(phxLoop) << "started emulation";
      } else {
        qCDebug(phxLoop) << "The emulator is already playing, rejecting play request";
      }

    } else if (requestType == "pauseEmu") {
      if (looperState != Playing) {
        stopTimer();
        qCDebug(phxLoop) << "paused emulation";
      } else {
        qCDebug(phxLoop) << "The emulator is not running, rejecting pause request";
      }
    }

  }

private:
  RestServer messageServer;
  CoreController coreController;
  QTimer emulationTimer;
};
