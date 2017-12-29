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

  qmlRegisterType<GameEntryDbModel>("vg.phoenix.models", 1, 0, "GameEntryDbModel");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/src/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  return app.exec();
}
