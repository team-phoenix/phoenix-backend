#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "gamemetadatadatabasemodel.h"
#include "gamemetadatadb.h"

#include <QDebug>

int main(int argc, char* argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  GameMetadataDB db;
  auto rows = db.findRomsBySha1("56FE858D1035DCE4B68520F457A0858BAE7BB16D");

  for (Rom &row : rows) {
    qDebug() << row.romID << row.romSize << row.systemID;
  }

  qmlRegisterType<GameMetadataDatabaseModel>("vg.phoenix.models", 1, 0, "GameMetadataDatabaseModel");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/src/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  return app.exec();
}
