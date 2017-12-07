#pragma once

#include <QString>
#include <QStringList>
#include <QByteArray>

struct RetroVariableValue {

  RetroVariableValue() = default;

  RetroVariableValue(QString value)
  {
    QStringList values = value.split(';');

    if (values.size() == 2) {
      choices = QString(values.last()).simplified().split('|');
      chosenValue = QString(choices.first()).toLocal8Bit();
      description = QString(values.first()).simplified();
      isValid = true;
    }
  }

  const char* getChosenValue() const
  {
    return chosenValue.constData();
  }

  QString description;
  QStringList choices;
  QByteArray chosenValue;
  bool isValid{ false };
};
