#pragma once

#include "messageserver.h"
#include "testing.h"

#include <QtTest>

#include <QDebug>
#include <QMutex>

static QMutex messageMutex;

class Test_MessageServer : public QObject {
    Q_OBJECT
    MessageServer *server;

    static bool wasCalled( const QVariantList &list ) {
        return list.size() == 1;
    }

private slots:

    void init() {
        messageMutex.lock();
        server = new MessageServer;
    }

    void cleanup() {
        delete server;
        messageMutex.unlock();
    }

    void should_emitBroadcastMessageWithCorrectJsonString() {

        const QJsonObject data {
            { "hello", "world" },
            { "apple", "orange" },
        };

        const QByteArray jsonStr = QJsonDocument( data ).toJson( QJsonDocument::Compact );

        QSignalSpy spy( server, &MessageServer::broadcastMessage );

        server->encodeMessage( data );

        QVariantList parameters = spy.at( 0 );

        QCOMPARE( jsonStr, parameters.at( 0 ).toByteArray() );
    }

    void should_emitInitEmuSignal() {

        const QJsonObject request {
            { "request", "initEmu" },
            { "core", "abc" },
            { "game", "123" },
            { "hardwareType", "world" },
        };


        QSignalSpy spy( server, &MessageServer::initEmu );

        server->parseJsonObject( request );

        QVariantList firstCallParameters = spy.takeFirst();

        QCOMPARE( firstCallParameters.at( 0 ).toString(), request[ "core" ].toString() );
        QCOMPARE( firstCallParameters.at( 1 ).toString(), request[ "game" ].toString() );
        QCOMPARE( firstCallParameters.at( 2 ).toString(), request[ "hardwareType" ].toString() );
    }

    void should_emitPlayEmuSignal() {

        const QJsonObject request {
            { "request", "playEmu" },
        };

        QSignalSpy spy( server, &MessageServer::playEmu );

        server->parseJsonObject( request );

        QCOMPARE( wasCalled( spy.takeFirst() ), true  );
    }

    void should_emitPauseEmuSignal() {

        const QJsonObject request {
            { "request", "pauseEmu" },
        };

        QSignalSpy spy( server, &MessageServer::pauseEmu );

        server->parseJsonObject( request );

        QCOMPARE( wasCalled( spy.takeFirst() ), true  );
    }

    void should_emitShutdownEmuSignal() {

        const QJsonObject request {
            { "request", "shutdownEmu" },
        };

        QSignalSpy spy( server, &MessageServer::shutdownEmu );

        server->parseJsonObject( request );

        QCOMPARE( wasCalled( spy.takeFirst() ), true  );
    }

    void should_emitRestartEmuSignal() {

        const QJsonObject request {
            { "request", "restartEmu" },
        };

        QSignalSpy spy( server, &MessageServer::restartEmu );

        server->parseJsonObject( request );

        QCOMPARE( wasCalled( spy.takeFirst() ), true  );
    }

    void should_emitKillEmuSignal() {

        const QJsonObject request {
            { "request", "killEmu" },
        };

        QSignalSpy spy( server, &MessageServer::killEmu );

        server->parseJsonObject( request );

        QCOMPARE( wasCalled( spy.takeFirst() ), true  );
    }

    void should_emitSaveStateSignal() {

        const QJsonObject request {
            { "request", "saveState" },
            { "path", "/path/to/save/the/state/at" },
        };

        QSignalSpy spy( server, &MessageServer::saveState );

        server->parseJsonObject( request );

        QCOMPARE( spy.takeFirst().first().toString(), request[ "path" ].toString() );
    }

    void should_emitUpdateVariableSignal() {

        const QJsonObject request {
            { "request", "updateVariable" },
            { "key", "a_random_key" },
            { "value", "a_random_value" },
        };

        QSignalSpy spy( server, &MessageServer::updateVariable );

        server->parseJsonObject( request );

        QVariantList firstCallParameters = spy.takeFirst();

        QCOMPARE( firstCallParameters.at( 0 ).toString(), request[ "key" ].toString() );
        QCOMPARE( firstCallParameters.at( 1 ).toString(), request[ "value" ].toString() );

    }

};
