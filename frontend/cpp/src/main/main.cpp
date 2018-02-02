#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "gamemetadatamodel.h"
#include "systemlistmodel.h"
#include "gameimporter.h"
#include "emulationlistener.h"
#include "emulationvideoscreen.h"
#include "pathcreator.h"
#include "globalinputdevice.h"

int main(int argc, char* argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  PathCreator::createAllAppPaths();

  qmlRegisterType<SystemModel>("vg.phoenix.models", 1, 0, "SystemModel");
  qmlRegisterType<GameImporter>("vg.phoenix.importer", 1, 0, "GameImporter");
  qmlRegisterType<EmulationVideoScreen>("vg.phoenix.emulation", 1, 0, "EmulationVideoScreen");

  QQmlApplicationEngine engine;
  QQmlContext* context = engine.rootContext();
  context->setContextProperty("globalGameMetadataModel", &GameMetadataModel::instance());
  context->setContextProperty("globalEmulationListener", &EmulationListener::instance());
  context->setContextProperty("globalInputDevice", &GlobalInputDevice::instance());

  engine.load(QUrl(QLatin1String("qrc:/src/main.qml")));

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  return app.exec();
}
