#include "gameimporter.h"
#include "logging.h"

#include <QCryptographicHash>
#include <QFile>
#include <QPair>

#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

using QPairList = QList<QPair<QString, QByteArray>>;

GameImporter::GameImporter(QObject* parent)
  : QObject(parent)
{
  connect(&importer, &Importer::importProgressChanged, this, [](qreal progress) {
    qDebug() << "import progress" << progress;
  });
  connect(&importer, &Importer::importCompleted, this, &GameImporter::updateModel);
  connect(&importer, &Importer::gameRemoved, this, &GameImporter::updateModel);
  connect(this, &GameImporter::updateModel, this, [] {qDebug() << "should update model now";});

  importer.moveToThread(&importerThread);
  importerThread.start();
}

GameImporter::~GameImporter()
{
  importerThread.quit();
  importerThread.wait();
}

void GameImporter::importGames(QList<QUrl> urls)
{
  QMetaObject::invokeMethod(&importer, "importFiles", Q_ARG(QList<QUrl>, urls));
}

void GameImporter::removeGameBySha1(QString sha1)
{
  QMetaObject::invokeMethod(&importer, "removeGameBySha1", Q_ARG(QString, sha1));
}

void GameImporter::stopImporterThread()
{
  importerThread.quit();
}


