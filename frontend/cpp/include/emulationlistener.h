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

public slots:
  void sendPlayMessage(QString gameFilePath, QString coreFilePath);
  void sendMessage(QVariantHash hashedMessage);

private:
  QVariantHash newMessage(const QString requestType)
  {
    return QVariantHash {
      { "request", requestType }
    };
  }

private:
  QLocalSocket socketToBackend;
  QLocalServer localServer;
  SocketReadWriter socketReadWriter;
};
