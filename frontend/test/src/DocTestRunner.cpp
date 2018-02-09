#include "DocTestRunner.h"

DocTestRunner::DocTestRunner(QObject* parent)
  : QObject(parent)
{
  docTest.moveToThread(&testThread);
  testThread.start();
}

void DocTestRunner::runTests(int argc, char** argv)
{
  QMetaObject::invokeMethod(&docTest, "runTests", Q_ARG(int, argc), Q_ARG(char**, argv));
}
