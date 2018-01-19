#pragma once

#include "socketreadwriter.h"

#include <QObject>
#include <QLocalServer>
#include <QJsonObject>

class EmulationListener : public QObject
{
  Q_OBJECT
public:
  explicit EmulationListener(QObject*  parent = nullptr);

  void executeSocketCommands(const QVariantHash &jsonMessage);

  void newConnectionFound();


  Q_INVOKABLE void sendPlayMessage(QString gameFilePath, QString coreFilePath)
  {
    qDebug() << "got play message" << gameFilePath << coreFilePath;

    if (gameFilePath.isEmpty() || coreFilePath.isEmpty()) {
      throw std::runtime_error("Game path and core path cannot be empty! aborting...");
    }

  }
private:
  QLocalServer localServer;
  SocketReadWriter socketReadWriter;
};
