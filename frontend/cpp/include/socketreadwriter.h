#pragma once

#include <QByteArray>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QObject>

class SocketReadWriter : public QObject
{
  Q_OBJECT
public:
  explicit SocketReadWriter(QObject* parent = nullptr);
  ~SocketReadWriter() = default;

  void readSocketMessage(QLocalSocket &socket);

signals:
  void newReplyFound(QVariantHash reply);

private:
  quint32 replySize{0};
};
