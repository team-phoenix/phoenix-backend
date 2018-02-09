#pragma once

#include "Importer.h"

#include <QObject>
#include <QUrl>
#include <QThread>

class QQmlEngine;
class QJSEngine;

class GameImporter : public QObject
{
  Q_OBJECT
public:
  explicit GameImporter(QObject* parent = nullptr);
  ~GameImporter();

signals:
  void updateModel();

public slots:
  void importGames(QList<QUrl> urls);
  void removeGameBySha1(QString sha1);

private slots:
  void stopImporterThread();

private:
  QThread importerThread;
  Importer importer;

};
