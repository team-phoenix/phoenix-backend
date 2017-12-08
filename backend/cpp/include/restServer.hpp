#pragma once
#include "logging.h"

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonDocument>

static const QString SERVER_NAME = "phoenixEmulatorProcess";

class RestServer : public QObject
{
  Q_OBJECT
public:
  RestServer()
  {
    QLocalServer::removeServer(SERVER_NAME);

    if (!localServer.listen(SERVER_NAME)) {
      qCDebug(phxServer, "QLocalSocket is not started...");
    } else {
      qCDebug(phxServer, "Backend server is listening");
    }

    QObject::connect(&localServer, &QLocalServer::newConnection, this,
                     &RestServer::handleNewConnection);

  }

  ~RestServer()
  {
    if (currentSocket) {
      currentSocket->deleteLater();
      currentSocket = nullptr;
    }
  }

  void sendRequest(QJsonObject request)
  {
    QByteArray requestBuffer = JSONObjectToByteArray(request);
    waitForDataWrite(requestBuffer);

    currentSocket->waitForBytesWritten();
  }

  void post(QJsonObject request)
  {

  }

  static QByteArray JSONObjectToByteArray(QJsonObject object)
  {
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
  }

  bool waitAndConnectNewSocket()
  {
    const int timeout = 3 * 1000;

    if (localServer.waitForNewConnection(timeout)) {
      handleNewConnection();
      return true;
    }

    return false;
  }

  void waitForDataWrite(const QByteArray &buffer)
  {
    quint32 size = static_cast<quint32>(buffer.size());
    const size_t writeSize = sizeof(size);

    qint64 wroteSize = 0;

    while (wroteSize < static_cast<qint64>(writeSize)) {

      auto wrote = currentSocket->write(reinterpret_cast<char*>(&size), writeSize);

      if (wrote == -1) {
        throw std::runtime_error(
          qPrintable(QString("There was an error with writing the buffer size to the socket: %1").arg(
                       QString::number(size))));
      } else {
        wroteSize += wrote;
      }
    }

    wroteSize = 0;

    while (wroteSize < buffer.size()) {

      auto wrote = currentSocket->write(buffer);

      if (wrote == -1) {
        throw std::runtime_error(
          qPrintable(QString("There was an error with writing data to the socket, data: %1").arg(
                       buffer.constData())));
      } else {
        wroteSize += wrote;
      }
    }

    currentSocket->flush();
  }

signals:
  void response(QJsonObject);

//signals:
//  void broadcastMessage(const QByteArray &t_message, bool waitUntilFinished = false);

//  void initEmu(const QString &core, const QString &game, const QString &hwType);
//  void playEmu();
//  void pauseEmu();
//  void shutdownEmu();
//  void restartEmu();

//  void killEmu();

//  void saveState(const QString &path);
//  void updateVariable(const QByteArray &t_key, const QByteArray &t_value);

//public slots:

//private slots:
//  void readSocket(QLocalSocket*);

private:
  QLocalServer localServer;
  QLocalSocket* currentSocket{ nullptr };

private slots:

  void readCurrentSocket()
  {
    if (currentSocket->bytesAvailable() > 4) {
      while (currentSocket->bytesAvailable()) {

        quint32 msgSize = 0;
        currentSocket->read(reinterpret_cast<char*>(&msgSize), sizeof(msgSize));


        if (currentSocket->bytesAvailable() >= msgSize) {
          QByteArray socketMsg(msgSize, '\0');

          currentSocket->read(socketMsg.data(), msgSize);

          const QJsonObject obj = QJsonDocument::fromBinaryData(socketMsg).object();
          parseResponse(obj);
        }
      }
    }
  }

  void handleNewConnection()
  {

    if (currentSocket != nullptr) {
      qDebug() << "Socket is already connected, not connecting a new one";
      return;
    }

    currentSocket = localServer.nextPendingConnection();

    qDebug() << "Connected to a new process.";

    connect(currentSocket, &QLocalSocket::readyRead, this, &RestServer::readCurrentSocket);

    connect(currentSocket, &QLocalSocket::connected, this, [this] {
      qCDebug(phxCore) << "localSocket is connected";
    });

    connect(currentSocket, &QLocalSocket::disconnected, this, [this] {
      qCDebug(phxCore) << "localSocket is disconnected";
      delete currentSocket;
      currentSocket = nullptr;
    });

  }

  void parseResponse(const QJsonObject &response)
  {
    qDebug() << "got response";
  }

//  void parseJsonObject(const QJsonObject &t_jsonObject);

};
