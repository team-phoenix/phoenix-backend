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
  connect(&importWatcher, &QFutureWatcher<void>::finished, this, &GameImporter::updateModel);
  connect(this, &GameImporter::updateModel, this, [] {qDebug() << "should update model now";});
}

void GameImporter::importGames(QList<QUrl> urls)
{
  QtConcurrent::blockingMap(urls, [this](const QUrl & url) -> QUrl {

    qDebug() << url.toString();
    const QString localFile = url.toString().replace("file:///", "");

    if (!QFile::exists(localFile))
    {
      return localFile;
    }
    QFile localFileObject(localFile);

    if (!localFileObject.open(QIODevice::ReadOnly))
    {
      throw std::runtime_error(qPrintable(localFile + " could not be opened"));
    }

    QCryptographicHash cryptoHash(QCryptographicHash::Sha1);

    if (!cryptoHash.addData(&localFileObject))
    {
      throw std::runtime_error(qPrintable(localFile + " could not be hashed successfully"));
    }

    const QByteArray hexHash = cryptoHash.result().toHex().toUpper();

    GameEntry gameEntry;
    gameEntry.absoluteFilePath = localFile;
    gameEntry.sha1Checksum = hexHash;
    libraryDb.insert(gameEntry);

    return localFile;
  });

  emit updateModel();

//  if (importWatcher.isRunning()) {
//    qDebug() << "Future is already running, disregarding the current operation";
//    return;
//  }

//  importWatcher.setFuture(mappedFuture);
}

void GameImporter::removeGameBySha1(QString sha1)
{
  libraryDb.removeBySha1(sha1);
}


