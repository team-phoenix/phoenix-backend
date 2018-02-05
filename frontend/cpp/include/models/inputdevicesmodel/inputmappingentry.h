#pragma once

#include <QString>

struct InputMappingEntry {

  InputMappingEntry(QString key, QString value)
  {
    mappingKey = key;
    mappingValue = value;
  }

  QString mappingKey;
  QString mappingValue;
};
