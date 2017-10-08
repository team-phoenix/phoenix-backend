#pragma once

#include <QObject>

#include <QLocalServer>

#include "testing.h"

class QLocalSocket;
class QJsonObject;


class MessageServer : public QObject
{
    Q_OBJECT
    friend class Test_MessageServer;
public:
    explicit MessageServer(QObject *parent = 0);

    MOCKABLE void encodeMessage( const QJsonObject &QJsonObject );

signals:
    void broadcastMessage( const QByteArray &t_message, bool waitUntilFinished = false );

    void initEmu( const QString &core, const QString &game, const QString &hwType );
    void playEmu( TEST_ARG_TREAT_AS_VOID );
    void pauseEmu( TEST_ARG_TREAT_AS_VOID );
    void shutdownEmu( TEST_ARG_TREAT_AS_VOID );
    void restartEmu( TEST_ARG_TREAT_AS_VOID );

    void killEmu( TEST_ARG_TREAT_AS_VOID );

    void saveState( const QString &path );
    void updateVariable( const QByteArray &t_key, const QByteArray &t_value );

public slots:

private slots:
    MOCKABLE void readSocket( QLocalSocket * );

private:
    QLocalServer m_localServer;

    MOCKABLE void parseJsonObject( const QJsonObject &t_jsonObject );

};
