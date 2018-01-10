#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "gameimporterintegrationtests.h"

#include <QCoreApplication>
#include <QTest>

QT_BEGIN_NAMESPACE
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS
QT_END_NAMESPACE

int runQTests(int argc, char* argv[])
{
  QTEST_ADD_GPU_BLACKLIST_SUPPORT

  const QList<QObject*> qTestList({
    new GameImporterIntegrationTests(),
  });

  int exec = 0;

  for (QObject* test : qTestList) {
    exec |= QTest::qExec(test, argc, argv);
  }

  QTEST_SET_MAIN_SOURCE_PATH
  return exec;
}

int main(int argc, char** argv)
{
  QGuiApplication app(argc, argv);
  app.setAttribute(Qt::AA_Use96Dpi, true);

  int execQTests = runQTests(argc, argv);
  int execCatchTests = Catch::Session().run(argc, argv);

  const int returnValue = execCatchTests | execQTests;

  if (returnValue == 0) {
    qInfo() << "All tests were successfully completed!";
  }

  return returnValue;
}
