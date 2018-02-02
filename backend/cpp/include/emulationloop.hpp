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

  static const int EMULATION_TIMER_INTERVAL_MILISECONDS = 16;
  static const int STANDBY_MODE_TIMER_INTERVAL_MILISECONDS = 32;

  enum LoopState {
    Standby,
    Initialized,
    Playing,
    Paused,
  };

  EmulationLoop()
  {
    connect(&messageServer, &RestServer::requestRecieved, this, &EmulationLoop::parseRequest);
    connect(&messageServer, &RestServer::socketDisconnected, this, &EmulationLoop::stopTimer);
    connect(&emulationTimer, &QTimer::timeout, this, &EmulationLoop::loop);

    connect(&standbyModeTimer, &QTimer::timeout, this, &EmulationLoop::pollInputManager);

  }

  void pollInputManager()
  {
    CoreController::inputPollCallback();
  }

  void enterStandbyMode()
  {
    if (looperState == Standby) {
      standbyModeTimer.start(STANDBY_MODE_TIMER_INTERVAL_MILISECONDS);
      qCDebug(phxLoop) << "The standbyMode timer has been activated";
    }
  }

  void stopStandbyMode()
  {
    standbyModeTimer.stop();
    qCDebug(phxLoop) << "The standbyMode timer has stopped";
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
  LoopState looperState{ Standby };
  CoreController::SystemInfo cachedInitSystemInfo;

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
    CoreController::instance().run();
  }

  void parseRequest(QJsonObject request)
  {
    const QString requestType = request.value("request").toString().simplified();
    qCDebug(phxLoop, "request %s", qPrintable(requestType));

    if (requestType == "initEmu") {
      if (looperState == Standby) {

        stopStandbyMode();

        const QString corePath = request[ "core" ].toString();
        const QString gamePath = request[ "game" ].toString();

        cachedInitSystemInfo = CoreController::instance().init(corePath,
                                                               gamePath);
        looperState = Initialized;

        messageServer.sendInitReply(cachedInitSystemInfo);

        qCDebug(phxLoop) << "initialized emulation";
      } else {
        qCDebug(phxLoop) << "The emulator is already initialized, rejecting initialize request";
        messageServer.sendInitReply(cachedInitSystemInfo);
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
  QTimer emulationTimer;
  QTimer standbyModeTimer;
};
