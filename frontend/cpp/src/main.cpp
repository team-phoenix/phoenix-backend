#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "gamemetadatadatabasemodel.h"

int main(int argc, char* argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  qmlRegisterType<GameMetadataDatabaseModel>("vg.phoenix.models", 1, 0, "GameMetadataDatabaseModel");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/src/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  return app.exec();
}
