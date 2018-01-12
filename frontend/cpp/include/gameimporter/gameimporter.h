#pragma once

#include "librarydb.h"
#include "openvgdb.h"

#include <QObject>
#include <QUrl>
#include <QFutureWatcher>

class QQmlEngine;
class QJSEngine;

class GameImporter : public QObject
{
  Q_OBJECT
public:
  explicit GameImporter(QObject* parent = nullptr);

signals:
  void updateModel();

public slots:
  void importGames(QList<QUrl> urls);
  void removeGameBySha1(QString sha1);

private:
  LibraryDb libraryDb;
  OpenVgDb openVgDb;
  QFutureWatcher<void> importWatcher;

  void mapFunction(const QUrl &url);
};
