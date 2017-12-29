#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "gameentrydbmodel.h"
#include "openvgdb.h"
#include "librarydb.h"

#include <QDebug>

int main(int argc, char* argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  LibraryDb libDb;
  libDb.insert(GameEntry(QVariantHash({
    { "absoluteFilePath", "4575474" },
    { "sha1Checksum", "fhhjh" },
  })));
  auto entries = libDb.findAllByGameEntry();

  for (GameEntry &row : entries) {
    qDebug() << row.rowIndex << row.absoluteFilePath << row.sha1Checksum;
  }

  OpenVgDb db;
  auto rows = db.findRomsBySha1("56FE858D1035DCE4B68520F457A0858BAE7BB16D");

  for (Rom &row : rows) {
    qDebug() << row.romID << row.romSize << row.systemID;
  }

  qmlRegisterType<GameEntryDbModel>("vg.phoenix.models", 1, 0, "GameEntryDbModel");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/src/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  return app.exec();
}
