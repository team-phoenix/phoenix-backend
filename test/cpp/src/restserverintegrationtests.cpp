#include "catch.hpp"

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

SCENARIO("")
{
  GIVEN("") {

    RestServer subject;

    QLocalSocket stubSocket;
    stubSocket.connectToServer(SERVER_NAME);

    subject.waitAndConnectNewSocket();

    if (!stubSocket.waitForConnected()) {
      qDebug() << "Socket Error:" << stubSocket.errorString();
      REQUIRE(false);
    }

    WHEN("") {
      const QJsonObject fakeRequest({{ "hello", "world" }});
      const auto expectedResponse = RestServer::JSONObjectToByteArray(fakeRequest);
      subject.sendRequest(fakeRequest);

      REQUIRE(stubSocket.waitForReadyRead());

      const auto actualResponse = waitForSocketReadAll(stubSocket);

      THEN("") {
        REQUIRE(actualResponse == expectedResponse);
      }
    }

    stubSocket.disconnectFromServer();
  }
}
