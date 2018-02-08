#include "doctest.hpp"

#include "restserver.hpp"

#include <QLocalSocket>

QByteArray waitForSocketReadAll(QLocalSocket &socket)
{
  if (socket.bytesAvailable() > 4) {
    while (socket.bytesAvailable()) {

      quint32 msgSize = 0;
      socket.read(reinterpret_cast<char*>(&msgSize), sizeof(msgSize));


      if (socket.bytesAvailable() >= msgSize) {
        QByteArray result(msgSize, '\0');

        socket.read(result.data(), msgSize);
        return result;
      }
    }
  }

  return QByteArray();
}

void sendRequest(QLocalSocket &socket, const QByteArray &buffer)
{
  quint32 size = static_cast<quint32>(buffer.size());
  const size_t writeSize = sizeof(size);

  qint64 wroteSize = 0;

  while (wroteSize < static_cast<qint64>(writeSize)) {

    auto wrote = socket.write(reinterpret_cast<char*>(&size), writeSize);

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

    auto wrote = socket.write(buffer);

    if (wrote == -1) {
      throw std::runtime_error(
        qPrintable(QString("There was an error with writing data to the socket, data: %1").arg(
                     buffer.constData())));
    } else {
      wroteSize += wrote;
    }
  }

  socket.flush();
}


SCENARIO("The separate process can send messages to the local server")
{
  GIVEN("a real local server") {

    RestServer subject;

    QLocalSocket stubSocket;
    stubSocket.connectToServer(SERVER_NAME);

    subject.waitAndConnectNewSocket();

    if (!stubSocket.waitForConnected()) {
      qDebug() << "Socket Error:" << stubSocket.errorString();
      REQUIRE(false);
    }

    WHEN("a request was sent to the listening server") {
      const QJsonObject fakeRequest({{ "hello", "world" }});
      const auto expectedResponse = RestServer::JSONObjectToByteArray(fakeRequest);
      subject.sendRequest(fakeRequest);

      REQUIRE(stubSocket.waitForReadyRead());

      const auto actualResponse = waitForSocketReadAll(stubSocket);

      THEN("the response can be read back") {
        REQUIRE(actualResponse.toStdString() == expectedResponse.toStdString());
      }
    }

    WHEN("a request was sent to the local server") {
      const QJsonObject fakeRequest({{ "joe", "schmo" }});
      const auto expectedRequest = RestServer::JSONObjectToByteArray(fakeRequest);

      sendRequest(stubSocket, expectedRequest);

      REQUIRE(subject.waitForRequest() == true);

      const auto actualResponse = subject.takeCurrentRequest();

      THEN("the request can be read back") {
        REQUIRE(actualResponse == fakeRequest);
      }
    }

    stubSocket.disconnectFromServer();
  }
}
