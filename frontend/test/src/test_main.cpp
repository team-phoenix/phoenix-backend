#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.hpp"

#include "gameimporterintegrationtests.h"

#include <QCoreApplication>
#include <QTest>

QT_BEGIN_NAMESPACE
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS
QT_END_NAMESPACE

static int EXEC = 0;

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

void runDocTest(int argc, char** argv)
{
  doctest::Context context;

  context.applyCommandLine(argc, argv);

  int exec = context.run();

  if (exec == 0) {
    qInfo() << "All tests were successfully completed!";
  }
}

int main(int argc, char** argv)
{
  QGuiApplication app(argc, argv);

//  int execQTests = runQTests(argc, argv);

  runDocTest(argc, argv);

  return 0;
}
