#pragma once

#include <QObject>
#include "doctest.hpp"

class DocTestObject : public QObject
{
  Q_OBJECT
public:
  explicit DocTestObject(QObject* parent = nullptr);

public slots:

  void runTests(int argc, char** argv);
};

Q_DECLARE_METATYPE(char**)
