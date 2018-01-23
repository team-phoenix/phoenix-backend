#pragma once
#include "logging.h"
#include "corecontroller.hpp"

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

  void sendInitReply(CoreController::SystemInfo systemInfo)
  {
    const retro_system_av_info systemAvInfo = systemInfo.avInfo;
    int pixelFormat = static_cast<int>(systemInfo.pixelFormat);

    QJsonObject replyObject {
      { "reply", "initEmu" },
      { "aspectRatio", systemAvInfo.geometry.aspect_ratio },
      { "height", static_cast<int>(systemAvInfo.geometry.base_height)},
      { "width", static_cast<int>(systemAvInfo.geometry.base_width)},
      { "frameRate", systemAvInfo.timing.fps},
      { "pixelFormat", pixelFormat}
    };

    sendReply(replyObject);
  }

  void sendPlayReply()
  {
    QJsonObject replyObject{
      { "reply", "playEmu" },
    };
    sendReply(replyObject);
  }

  void sendPausedReply()
  {
    QJsonObject replyObject{
      { "reply", "pausedEmu" },
    };
    sendReply(replyObject);
  }

  void sendReply(QJsonObject replyObject)
  {
    const QByteArray replyBuffer = JSONObjectToByteArray(replyObject);
    waitForDataWrite(replyBuffer);
  }

  void sendRequest(QJsonObject request)
  {
    QByteArray requestBuffer = JSONObjectToByteArray(request);
    waitForDataWrite(requestBuffer);

    currentSocket->waitForBytesWritten();
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

  bool requestIsPending() const
  {
    const int finishImmediately = 0;
    return currentSocket->waitForReadyRead(finishImmediately);
  }

  bool waitForRequest()
  {
    return currentSocket->waitForReadyRead();
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

  void readCurrentSocket()
  {

    while (currentSocket->bytesAvailable()) {
      if (currentSocket->bytesAvailable() < 4) {
        return;
      }

      if (currentMessageSize == 0) {
        currentSocket->read(reinterpret_cast<char*>(&currentMessageSize), sizeof(currentMessageSize));
      }

      if (currentMessageSize != 0 && currentSocket->bytesAvailable() >= currentMessageSize) {

        QByteArray incomingMessage(currentMessageSize, '\0');

        int bytesRead = 0;

        while (bytesRead < incomingMessage.size()) {
          bytesRead += currentSocket->read(incomingMessage.data() + bytesRead,
                                           incomingMessage.size() - bytesRead);
        }

        QJsonParseError parserError;
        const QJsonObject requestJsonObject = QJsonDocument::fromJson(incomingMessage,
                                                                      &parserError).object();

        if (parserError.error != QJsonParseError::NoError) {
          throw std::runtime_error(qPrintable(QString("Could not parse the JSON request: ") +
                                              parserError.errorString()));
        }

        currentMessageSize = 0;
        currentReadObject = requestJsonObject;
        emit requestRecieved(requestJsonObject);
      }

    }
  }

  int readMessageSize()
  {
    quint32 msgSize = 0;
    currentSocket->read(reinterpret_cast<char*>(&msgSize), sizeof(msgSize));
    return msgSize;
  }

  QJsonObject takeCurrentRequest()
  {
    QJsonObject readObject = currentReadObject;
    currentReadObject = QJsonObject();
    return readObject;
  }

signals:
  void requestRecieved(QJsonObject);
  void socketDisconnected();
  void socketConnected();

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
  QJsonObject currentReadObject;
  int currentMessageSize{0};

private slots:

  void handleNewConnection()
  {

    if (currentSocket != nullptr) {
      qDebug() << "Socket is already connected, not connecting a new one";
      return;
    }

    currentSocket = localServer.nextPendingConnection();

    qDebug() << "Connected to a new process.";

    connect(currentSocket, &QLocalSocket::readyRead, this, &RestServer::readCurrentSocket);

    connect(currentSocket, &QLocalSocket::connected, this, &RestServer::socketConnected);

    connect(currentSocket, &QLocalSocket::connected, this, [this] {
      qCDebug(phxCore) << "localSocket is connected";
    });

    connect(currentSocket, &QLocalSocket::disconnected, this, [this] {
      qCDebug(phxCore) << "localSocket is disconnected";
      currentSocket->deleteLater();
      currentSocket = nullptr;
      emit socketDisconnected();
    });

  }


//  void parseJsonObject(const QJsonObject &t_jsonObject);

};
