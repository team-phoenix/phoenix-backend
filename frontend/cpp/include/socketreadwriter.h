#pragma once

#include <QByteArray>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

class SocketReadWriter
{
public:
  SocketReadWriter() = default;
  ~SocketReadWriter() = default;

  QVariantHash readSocketMessage(QLocalSocket &socket)
  {
    QVariantHash result;

    // The first 4 bytes are reserved for the size of the message,
    // so we can skip the readyRead signal if the buffer is < 4 bytes.
    if (socket.bytesAvailable() > 4) {
      while (socket.bytesAvailable()) {

        quint32 msgSize = 0;
        socket.read(reinterpret_cast<char*>(&msgSize), sizeof(msgSize));

        if (socket.bytesAvailable() >= msgSize) {
          QByteArray socketMsg(msgSize, '\0');

          socket.read(socketMsg.data(), msgSize);

          qDebug() << socketMsg;
          QJsonObject obj = QJsonDocument::fromJson(socketMsg).object();
          result = obj.toVariantHash();
          break;
        }
      }

    }

    return result;
  }

};
