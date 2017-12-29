#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <QCoreApplication>

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);
  Q_UNUSED(app);
  return Catch::Session().run(argc, argv);
}
