#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.hpp"

#include <QDebug>

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
  runDocTest(argc, argv);
  return 0;
}
