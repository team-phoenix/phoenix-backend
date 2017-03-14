#include "messageserver.h"
#include "logging.h"

#include <QJsonObject>
#include <QJsonDocument>

#include <QLocalSocket>

MessageServer::MessageServer(QObject *parent) : QObject(parent)
{
    const QString serverName = "phoenixEmulatorProcess";
    QLocalServer::removeServer(serverName);
    if ( !m_localServer.listen( serverName ) ) {
        qCDebug( phxServer, "QLocalSocket is not started..." );
    } else {
        qCDebug(phxServer, "Backend server is listening" );
    }

    connect( &m_localServer, &QLocalServer::newConnection, this, [this] {
        QLocalSocket *socket = m_localServer.nextPendingConnection();

        qDebug() << "Connected to a new process.";

        // Listen to incoming pipes from external processes, we can we
        // fulfill their request.
        connect( socket, &QLocalSocket::readyRead, this, [ this, socket ] { readSocket( socket ); });

        // Connect to a frontend server, so we can request information from it.
        QLocalSocket *outSocket = new QLocalSocket( this );
        outSocket->connectToServer( "phoenixExternalProcess" );

        connect( this, &MessageServer::broadcastMessage, this, [this, outSocket] ( const QByteArray &t_data, bool waitUntilFinished ) {
            quint32 size = static_cast<quint32>( t_data.size() );
            outSocket->write( reinterpret_cast<char *>( &size ), sizeof( size ) );
            outSocket->write( t_data );
            outSocket->flush();
            if ( waitUntilFinished ) {
                outSocket->waitForBytesWritten();
            }
        });

        connect( outSocket, &QLocalSocket::connected, this, [this] {
           qCDebug( phxCore ) << "Connected to Frontend";
        });

    });
}

void MessageServer::encodeMessage(const QJsonObject &t_message ) {
    emit broadcastMessage( QJsonDocument( t_message ).toJson( QJsonDocument::Compact ) );
}

void MessageServer::readSocket( QLocalSocket *t_socket ) {
    if ( t_socket->bytesAvailable() > 4 ) {
        while ( t_socket->bytesAvailable() ) {

            quint32 msgSize = 0;
            t_socket->read( reinterpret_cast<char *>( &msgSize ), sizeof( msgSize ) );


            if ( t_socket->bytesAvailable() >= msgSize ) {
                QByteArray socketMsg( msgSize, '\0' );

                t_socket->read( socketMsg.data(), msgSize );

                QJsonObject obj = QJsonDocument::fromBinaryData( socketMsg ).object();
                parseJsonObject( obj );
            }
        }
    }
}

void MessageServer::parseJsonObject(const QJsonObject &t_jsonObject) {

    if ( !t_jsonObject.isEmpty() ) {

        const QString request = t_jsonObject[ "request" ].toString().simplified();

        qCDebug( phxServer, "request %s", qPrintable( request ) );

        if ( request == "initEmu" ) {
            const QString core = t_jsonObject[ "core" ].toString();
            const QString game = t_jsonObject[ "game" ].toString();
            const QString hwType = t_jsonObject[ "hardwareType" ].toString();

            emit initEmu( core, game, hwType );

        } else if ( request == "killEmu" ) {
            emit killEmu();

        } else if ( request == "playEmu" ) {
            emit playEmu();

        } else if ( request == "pauseEmu" ) {
            emit pauseEmu();

        } else if ( request == "restartEmu" ) {
            emit restartEmu();

        } else if ( request == "shutdownEmu" ) {
            emit shutdownEmu();

        } else if ( request == "saveState" ) {

            const QString path = t_jsonObject[ "path" ].toString();

            emit saveState( path );

        }
    }
}
