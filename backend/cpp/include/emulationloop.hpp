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
    const QString requestType = request.value("request").toString().simplified();
    qCDebug(phxLoop, "request %s", qPrintable(requestType));

    if (requestType == "initEmu") {
      if (looperState == Uninitialized) {
        const QString corePath = request[ "core" ].toString();
        const QString gamePath = request[ "game" ].toString();

        const CoreController::SystemInfo systemInfo = coreController.init(corePath,
                                                                          gamePath);
        looperState = Initialized;

        messageServer.sendInitReply(systemInfo);

        qCDebug(phxLoop) << "initialized emulation";
      } else {
        qCDebug(phxLoop) << "The emulator is already initialized, rejecting initialize request";
      }
    } else if (requestType == "playEmu") {

      if (looperState == Initialized) {

        loop();
        messageServer.sendPlayReply();
        qCDebug(phxLoop) << "started emulation";

        looperState = Playing;
        const int miliseconds = 16;
        startTimerWithInterval(miliseconds);

      } else if (looperState == Paused) {
        qCDebug(phxLoop) << "The paused game will resume playing...";
        messageServer.sendPlayReply();
        startTimer();
      } else {
        qCDebug(phxLoop) << "The emulator is already playing, rejecting play request";
      }

    } else if (requestType == "pauseEmu") {
      if (looperState != Playing) {
        stopTimer();
        messageServer.sendPausedReply();
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
