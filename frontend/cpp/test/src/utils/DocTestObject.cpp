#include "DocTestObject.h"

DocTestObject::DocTestObject(QObject* parent)
  : QObject(parent)
{

}

void DocTestObject::runTests(int argc, char** argv)
{
  doctest::Context context;
  context.applyCommandLine(argc, argv);

  int exec = context.run();
  exit(exec);
}
