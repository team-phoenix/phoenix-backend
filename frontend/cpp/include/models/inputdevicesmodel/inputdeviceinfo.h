#pragma once

#include <QString>
#include <QVariantHash>

struct InputDeviceInfo {

  InputDeviceInfo(QString name, int port, QVariantHash mapping)
  {
    inputDeviceName = name;
    inputDevicePort = port;
    inputMapping = mapping;
  }

  QString inputDeviceName;
  int inputDevicePort{ -1 };
  QVariantHash inputMapping;
};
