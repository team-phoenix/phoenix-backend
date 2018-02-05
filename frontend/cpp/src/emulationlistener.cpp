#include "emulationlistener.h"
#include "pathcreator.h"

#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

static const QString SERVER_NAME = "phoenixEmulatorProcess";

EmulationListener::EmulationListener(QObject* parent)
  : QObject(parent)
{

  if (!QLocalServer::removeServer(SERVER_NAME))  {
    throw std::runtime_error(qPrintable(QString("The %1 server could not be removed, aborting...").arg(
                                          SERVER_NAME)));
  }

  qDebug() << "Frontend server is listening...";

  connect(&socketReadWriter, &SocketReadWriter::newReplyFound, this,
          &EmulationListener::executeSocketCommands);

  connect(&socketToBackend, &QLocalSocket::connected, this, [this] {
    qDebug() << "Connected socket to backend";
  });

  connect(&socketToBackend, &QLocalSocket::readyRead, this, [this] {
    socketReadWriter.readSocketMessage(socketToBackend);
  });

  socketToBackend.connectToServer(SERVER_NAME);
}

void EmulationListener::newConnectionFound()
{
  QLocalSocket* newSocket = localServer.nextPendingConnection();

  connect(newSocket, &QLocalSocket::readyRead, this, [this, newSocket] {
    socketReadWriter.readSocketMessage(*newSocket);
  });

  connect(newSocket, &QLocalSocket::disconnected, this, [newSocket] {
    delete newSocket;
  });
}

QVariantHash EmulationListener::newMessage(const QString requestType)
{
  return QVariantHash {
    { "request", requestType }
  };
}

EmulationListener &EmulationListener::instance()
{
  static EmulationListener emulationListener;
  return emulationListener;
}

void EmulationListener::getInputInfoList()
{
  const QVariantHash inputInfoMessage = newMessage("getInputDeviceInfoList");
  sendMessage(inputInfoMessage);
}

bool EmulationListener::sendPlayMessage(QString gameFilePath, QString gameSystem)
{

  if (gameFilePath.isEmpty() || gameSystem.isEmpty()) {
    throw std::runtime_error("Game path and system cannot be empty! aborting...");
  }

  const QString core = systemDb.findCoreWhereSystemFullNameIs(gameSystem);

  const QString fullCorePath = PathCreator::addExtension(PathCreator::corePath() + core);


  if (!QFile::exists(fullCorePath)) {
    qDebug() << fullCorePath << "doesn't exist, not sending play message";
    return false;
  }

  QVariantHash initMessage = newMessage("initEmu");
  initMessage["core"] = fullCorePath;
  initMessage["game"] = gameFilePath;

  sendMessage(initMessage);

  const QVariantHash playMessage = newMessage("playEmu");
  sendMessage(playMessage);
  return true;
}

void EmulationListener::sendMessage(QVariantHash hashedMessage)
{
  if (socketToBackend.isOpen()) {
    QByteArray messageBytes = QJsonDocument::fromVariant(hashedMessage).toJson(
                                QJsonDocument::Compact);

    quint32 size = static_cast<quint32>(messageBytes.size());

    socketToBackend.write(reinterpret_cast<char*>(&size), sizeof(size));

    socketToBackend.write(messageBytes);

  }
}


void EmulationListener::executeSocketCommands(QVariantHash replyMessage)
{
  const QString replyType = replyMessage.value("reply").toString().simplified();

  if (replyType == "initEmu") {

    const double aspectRatio = replyMessage.value("aspectRatio").toDouble();
    const int height = replyMessage.value("height").toInt();
    const int width = replyMessage.value("width").toInt();
    const double frameRate = replyMessage.value("frameRate").toDouble();
    const int pixelFormat = replyMessage.value("pixelFormat").toInt();

    emit videoInfoChanged(aspectRatio, height, width, frameRate, pixelFormat);

  } else if (replyType == "playEmu") {

    emit startReadingFrames();

  } else if (replyType == "pausedEmu") {

    emit pauseReadingFrames();

  } else if (replyType == "inputStateUpdate") {

    const int port = replyMessage.value("port").toInt();
    const int id = replyMessage.value("id").toInt();
    const int state = replyMessage.value("state").toInt();

    emit inputStateUpdated(port, id, state);

  } else if (replyType == "inputDeviceInfoList") {
    const QJsonArray devices = replyMessage.value("deviceInfoList").toJsonArray();

    QList<InputDeviceInfo> deviceInfo;

    for (const QJsonValue &val : devices) {

      QJsonObject deviceObject = val.toObject();
      const QVariantHash deviceMapping = deviceObject.value("deviceMapping").toVariant().toHash();

      deviceInfo.append(InputDeviceInfo(deviceObject.value("deviceName").toString(),
                                        deviceObject.value("devicePort").toInt()
                                        , deviceMapping));
    }

    emit inputInfoListRecieved(deviceInfo);

  } else {
    qDebug() << "Unhandled socket command" << replyMessage;
  }
}

/*
 * // Tells the sandboxed emulator process to shutdown once the frontend has quit.
  QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [ this ] {
    sendProcessMessage(QJsonObject{ { "request", "killEmu" } });
  });

  const QString serverName = "phoenixExternalProcess";

  // Construct a new server instance, while removing any old server files that may exist
  // on the storage device, from a previous instance.
  QLocalServer* server = new QLocalServer();
  QLocalServer::removeServer(serverName);

  if (!server->listen(serverName)) {
    qDebug() << "Could not open server:" << serverName;
    return;
  } else {
    qDebug() << "Opened up the frontend server";
  }


  // Make the new server connect to and listen to any incoming sockets, this way we can tracks all listeners.
  // Most of the time there will only ever be one listener, the frontend, but in the future, we may get more, which could be quite handy.

  connect(server, &QLocalServer::newConnection, this, [this, server] {
    QLocalSocket* socket = server->nextPendingConnection();

    connect(socket, &QLocalSocket::readyRead, this, [this, socket] {

      // The first 4 bytes are reserved for the size of the message,
      // so we can skip the readyRead signal if the buffer is < 4 bytes.
      if (socket->bytesAvailable() > 4)
      {
        while (socket->bytesAvailable()) {

          quint32 msgSize = 0;
          socket->read(reinterpret_cast<char*>(&msgSize), sizeof(msgSize));

          if (socket->bytesAvailable() >= msgSize) {
            QByteArray socketMsg(msgSize, '\0');

            socket->read(socketMsg.data(), msgSize);

            qDebug() << socketMsg;
            QJsonObject obj = QJsonDocument::fromJson(socketMsg).object();
            readProcessMessage(obj);
          }
        }

      }
    });

    qDebug() << "Found a listening process";

    emit newProcessConnected();

  });

  // Construct a socket that enables us to listen to the sandboxed emulator process by connecting to its server.

  QLocalSocket* socket = new QLocalSocket(this);

  connect(this, &EmulatorManager::pipeMessage, this, [this, socket](QByteArray t_request) {
    if (socket->isOpen()) {
      quint32 size = static_cast<quint32>(t_request.size());
      qDebug() << "size" << size << t_request;
      socket->write(reinterpret_cast<char*>(&size), sizeof(size));
      socket->write(t_request);
      socket->flush();
    }
  });

  connect(socket, &QLocalSocket::connected, this, [this]() {
    qDebug() << "connected";
    const QString core = "C:/Users/lee/Desktop/bsnes_balanced_libretro.dll";
    const QString game =
      "C:/Users/lee/Documents/cpp/phoenix-backend/test/snes_test_roms/bsnesdemo_v1.sfc";
    initEmu(core, game, "2d");
    playEmu();
  });
  connect(socket, &QLocalSocket::connected, this, &EmulatorManager::connectedToProcess);
  connect(socket, &QLocalSocket::disconnected, this, &EmulatorManager::disconnectedFromProcess);
  connect(socket, &QLocalSocket::disconnected, this, [this] {
    qDebug() << "Frontend socket disconnected";
  });

  socket->connectToServer("phoenixEmulatorProcess");

 */
