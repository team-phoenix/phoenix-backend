#include "socketreadwriter.h"

SocketReadWriter::SocketReadWriter(QObject* parent)
  : QObject(parent)
{

}

void SocketReadWriter::readSocketMessage(QLocalSocket &socket)
{

  while (socket.bytesAvailable()) {

    if (socket.bytesAvailable() < 4) {
      return;
    }

    if (replySize == 0) {
      socket.read(reinterpret_cast<char*>(&replySize), sizeof(replySize));
    }

    if (replySize != 0 && socket.bytesAvailable() >= replySize) {

      QByteArray replyBuffer(replySize, '\0');

      int bytesRead = 0;

      while (bytesRead < replyBuffer.size()) {
        bytesRead += socket.read(replyBuffer.data() + bytesRead, replyBuffer.size() - bytesRead);
      }

      QJsonParseError parserError;
      const QJsonObject replyJsonObject = QJsonDocument::fromJson(replyBuffer, &parserError).object();

      if (parserError.error != QJsonParseError::NoError) {
        throw std::runtime_error(qPrintable(QString("Could not parse the JSON request: ") +
                                            parserError.errorString()));
      }

      replySize = 0;
      emit newReplyFound(replyJsonObject.toVariantHash());
    }
  }

}
