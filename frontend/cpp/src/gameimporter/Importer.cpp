#include "Importer.h"

#include <QtConcurrent>

Importer::Importer(QObject* parent)
  : QObject(parent)
{

}

void Importer::importFiles(QList<QUrl> files)
{
  const qreal totalFiles = static_cast<qreal>(files.size());
  int i = 1;

  QtConcurrent::blockingMap(files, [this, &i, &totalFiles](const QUrl & fileUrl) {

    const QString localFile = fileUrl.toString().replace("file:///", "");

    if (!QFile::exists(localFile)) {
      return;
    }

    QFile localFileObject(localFile);

    if (!localFileObject.open(QIODevice::ReadOnly)) {
      throw std::runtime_error(qPrintable(localFile + " could not be opened"));
    }

    QCryptographicHash cryptoHash(QCryptographicHash::Sha1);

    if (!cryptoHash.addData(&localFileObject)) {
      throw std::runtime_error(qPrintable(localFile + " could not be hashed successfully"));
    }

    const QByteArray hexHash = cryptoHash.result().toHex().toUpper();

    GameEntry gameEntry;
    gameEntry.absoluteFilePath = localFile;
    gameEntry.sha1Checksum = hexHash;

    const QPair<Release, System> releaseSystemPair = openVgDb.findReleasesByTitleWithBestGuess(
                                                       gameEntry.absoluteFilePath);

    const Release release = releaseSystemPair.first;
    gameEntry.gameDescription = release.releaseDescription;
    gameEntry.gameImageSource = release.releaseCoverFront;
    gameEntry.gameTitle = release.releaseTitleName;

    const System system = releaseSystemPair.second;
    gameEntry.systemFullName = system.systemName;

    libraryDb.insert(gameEntry);

    emit importProgressChanged(i / totalFiles);
    ++i;
  });

  emit importCompleted();
}

void Importer::removeGameBySha1(QString sha1)
{
  libraryDb.removeBySha1(sha1);
  emit gameRemoved();
}
