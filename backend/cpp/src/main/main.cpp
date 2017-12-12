#include "emulationloop.hpp"

#include <QCoreApplication>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  EmulationLoop emulationLoop;
  Q_UNUSED(emulationLoop);

  return app.exec();
}


