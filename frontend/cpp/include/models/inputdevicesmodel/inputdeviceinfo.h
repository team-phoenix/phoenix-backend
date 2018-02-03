#pragma once

#include <QString>

struct InputDeviceInfo {

  InputDeviceInfo(QString name, int port)
  {
    inputDeviceName = name;
    inputDevicePort = port;
  }

  QString inputDeviceName;
  int inputDevicePort{ -1 };
};
