#pragma once

#include "DocTestObject.h"

#include <QObject>
#include <QThread>

class DocTestRunner : public QObject
{
  Q_OBJECT
public:
  explicit DocTestRunner(QObject* parent = nullptr);

  void runTests(int argc, char** argv);

private:
  QThread testThread;
  DocTestObject docTest;
};
