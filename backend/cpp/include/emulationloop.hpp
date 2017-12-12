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
  EmulationLoop()
  {
    connect(&messageServer, &RestServer::requestRecieved, this, &EmulationLoop::parseRequest);
    connect(&messageServer, &RestServer::socketDisconnected, this, &EmulationLoop::stop);
    connect(&emulationTimer, &QTimer::timeout, this, &EmulationLoop::loop);
  }

  void startTimerWithInterval(int miliseconds)
  {
    stop();
    emulationTimer.setTimerType(Qt::PreciseTimer);
    emulationTimer.setInterval(miliseconds);
    start();
  }

  ~EmulationLoop() = default;

private slots:

  void stop()
  {
    emulationTimer.stop();
    qCDebug(phxLoop) << "timer was stopped";
  }

  void start()
  {
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
      const QString corePath = request[ "core" ].toString();
      const QString gamePath = request[ "game" ].toString();
      coreController.init(corePath, gamePath);
      qCDebug(phxLoop) << "initialized emulation";

    } else if (requestType == "playEmu") {

      const int miliseconds = 16;
      startTimerWithInterval(miliseconds);
      qCDebug(phxLoop) << "started emulation";

    } else if (requestType == "pauseEmu") {
      stop();
      qCDebug(phxLoop) << "paused emulation";
    }

  }

private:
  RestServer messageServer;
  CoreController coreController;
  QTimer emulationTimer;
};
