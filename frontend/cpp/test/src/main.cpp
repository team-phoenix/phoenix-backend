#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.hpp"

#include "DocTestRunner.h"
#include "GameImporterIntegrationTests.h"

#include <QCoreApplication>

int main(int argc, char** argv)
{
  QGuiApplication app(argc, argv);

  qRegisterMetaType<char**>();

  DocTestRunner docTestRunner;
  docTestRunner.runTests(argc, argv);

  return app.exec();
}
