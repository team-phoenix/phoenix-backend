#pragma once

#include "librarydb.h"

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
  void importGames(const QList<QUrl> &urls);

private:
  LibraryDb libraryDb;
  QFutureWatcher<void> importWatcher;

  void mapFunction(const QUrl &url);
};
