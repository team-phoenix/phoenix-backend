#pragma once

#include "socketreadwriter.h"
#include "systemdb.h"
#include "inputdeviceinfo.h"

#include <QObject>
#include <QLocalServer>
#include <QJsonObject>

class EmulationListener : public QObject
{
  Q_OBJECT
public:

  static EmulationListener &instance();

public slots:
  void getInputInfoList();
  bool sendPlayMessage(QString gameFilePath, QString gameSystem);
  void sendMessage(QVariantHash hashedMessage);

private slots:
  void executeSocketCommands(QVariantHash jsonMessage);

private:
  void newConnectionFound();

  QVariantHash newMessage(const QString requestType);

signals:
  void videoInfoChanged(double, int, int, double, int);
  void startReadingFrames();
  void pauseReadingFrames();
  void inputStateUpdated(int port, int id, int state);
  void inputInfoListRecieved(QList<InputDeviceInfo> inputInfoList);

private:
  QLocalSocket socketToBackend;
  QLocalServer localServer;
  SocketReadWriter socketReadWriter;
  SystemDb systemDb;

  explicit EmulationListener(QObject*  parent = nullptr);

};
