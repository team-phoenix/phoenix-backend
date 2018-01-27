#pragma once

#include "socketreadwriter.h"
#include "systemdb.h"

#include <QObject>
#include <QLocalServer>
#include <QJsonObject>

class EmulationListener : public QObject
{
  Q_OBJECT
public:

  static EmulationListener &instance();

public slots:
  bool sendPlayMessage(QString gameFilePath, QString gameSystem);
  void sendMessage(QVariantHash hashedMessage);

private slots:
  void executeSocketCommands(QVariantHash jsonMessage);

private:
  void newConnectionFound();

  QVariantHash newMessage(const QString requestType)
  {
    return QVariantHash {
      { "request", requestType }
    };
  }

signals:
  void videoInfoChanged(double, int, int, double, int);
  void startReadingFrames();
  void pauseReadingFrames();

private:
  QLocalSocket socketToBackend;
  QLocalServer localServer;
  SocketReadWriter socketReadWriter;
  SystemDb systemDb;

  explicit EmulationListener(QObject*  parent = nullptr);

};
