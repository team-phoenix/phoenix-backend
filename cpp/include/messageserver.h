#pragma once

#include <QObject>

#include <QLocalServer>

class QLocalSocket;
class QJsonObject;

class MessageServer : public QObject
{
    Q_OBJECT
public:
    explicit MessageServer(QObject *parent = 0);

    void encodeMessage( const QJsonObject &QJsonObject );

signals:
    void broadcastMessage( const QByteArray &t_message, bool waitUntilFinished = false );

    void initEmu( const QString &core, const QString &game, const QString &hwType );
    void playEmu();
    void pauseEmu();
    void shutdownEmu();
    void killEmu();

    void saveState( const QString &path );

public slots:

private slots:
    void readSocket( QLocalSocket * );

private:
    QLocalServer m_localServer;

    void parseJsonObject( const QJsonObject &t_jsonObject );

};
