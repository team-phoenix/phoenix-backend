#pragma once

#include "librarydb.h"
#include "openvgdb.h"

#include <QObject>
#include <QList>
#include <QUrl>

class Importer : public QObject
{
  Q_OBJECT
public:
  explicit Importer(QObject* parent = nullptr);

public slots:
  void importFiles(QList<QUrl> files);
  void removeGameBySha1(QString sha1);

signals:
  void importProgressChanged(qreal progress);
  void importCompleted();
  void gameRemoved();

private:
  LibraryDb libraryDb;
  OpenVgDb openVgDb;
};
