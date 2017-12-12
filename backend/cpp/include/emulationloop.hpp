#pragma once

#include "corecontroller.hpp"
#include "restServer.hpp"

#include <QObject>
#include <QTimer>

class EmulationLoop : public QObject
{
  Q_OBJECT
public:
  EmulationLoop()
  {
    connect(&messageServer, &RestServer::requestRecieved, this, &EmulationLoop::parseRequest);
    connect(&emulationTimer, &QTimer::timeout, this, &EmulationLoop::loop);
  }

  void start()
  {
    emulationTimer.start();
  }

  void stop()
  {
    emulationTimer.stop();
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

  void loop()
  {
    coreController.run();
  }

  void parseRequest(QJsonObject request)
  {
    for (auto iter = request.begin(); iter != request.end(); ++iter) {
      const QString key = iter.key();

      if (key == "initEmu") {
        const QString corePath = request[ "core" ].toString();
        const QString gamePath = request[ "game" ].toString();
        coreController.init(corePath, gamePath);
      } else if (key == "playEmu") {
        const int miliseconds = 16;
        startTimerWithInterval(miliseconds);
      }
    }
  }

private:
  RestServer messageServer;
  CoreController coreController;
  QTimer emulationTimer;
};
