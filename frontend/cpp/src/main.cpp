#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "gamemetadatamodel.h"
#include "systemlistmodel.h"

int main(int argc, char* argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  qmlRegisterType<GameMetadataModel>("vg.phoenix.models", 1, 0, "GameMetadataModel");
  qmlRegisterType<SystemModel>("vg.phoenix.models", 1, 0, "SystemModel");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/src/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  return app.exec();
}
