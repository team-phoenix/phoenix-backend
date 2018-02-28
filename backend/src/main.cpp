#include "EmulationLoop.hpp"

#include <QCoreApplication>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  EmulationLoop emulationLoop;
  emulationLoop.enterStandbyMode();

  return app.exec();
}


