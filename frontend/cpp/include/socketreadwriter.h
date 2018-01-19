#pragma once

#include <QByteArray>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

template<typename SomeSocket = QLocalSocket>
class SocketReadWriter_T
{
public:
  SocketReadWriter_T() = default;
  ~SocketReadWriter_T() = default;

  QVariantHash readSocketMessage(SomeSocket &socket)
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

using SocketReadWriter = SocketReadWriter_T<>;
