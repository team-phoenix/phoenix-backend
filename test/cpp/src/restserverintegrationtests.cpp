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

    WHEN("a request was sent to the local server") {
      const QJsonObject fakeRequest({{ "hello", "world" }});
      const auto expectedResponse = RestServer::JSONObjectToByteArray(fakeRequest);
      subject.sendRequest(fakeRequest);

      REQUIRE(stubSocket.waitForReadyRead());

      const auto actualResponse = waitForSocketReadAll(stubSocket);

      THEN("the response can be read back") {
        REQUIRE(actualResponse == expectedResponse);
      }
    }

    stubSocket.disconnectFromServer();
  }
}
