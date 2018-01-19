#include "catch.hpp"
#include "emulationlistener.h"
#include "socketreadwriter.h"

#include <QLocalSocket>
#include <QLocalServer>

SCENARIO("EmulationListener")
{
  GIVEN("a mocked out subject") {
//    EmulationListener subject;

//    WHEN("a new socket emits a readReady signal") {
//      struct MockLocalSocket : public QLocalSocket {
//        ~MockLocalSocket() = default;
//      };

//      MockLocalSocket mockLocalSocket;

//      struct MockLocalServer : public QLocalServer {
//        ~MockLocalServer() = default;

//        QLocalSocket* nextPendingConnection()
//        {
//          return  &mockLocalSocket;
//        }
//      };

//      struct MockSocketReadWriter : public SocketReadWriter {

//      };

//      MockLocalServer mockLocalServer;
//      subject.setLocalServer(mockLocalServer);

//      THEN("the subject is able to read the entirety of the message") {
//        emit mockLocalServer.readReady();


//      }
//    }
  }
}
