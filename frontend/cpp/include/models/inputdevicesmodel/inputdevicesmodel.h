#pragma once

#include "inputdeviceinfo.h"

#include <QAbstractListModel>
#include <QList>

class InputDeviceInfoModel : public QAbstractListModel
{
  Q_OBJECT
public:

  enum Roles {
    InputDeviceName = Qt::UserRole + 1,
    InputDevicePort,
    InputDeviceDisplayName,
  };

  explicit InputDeviceInfoModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const;

  int columnCount(const QModelIndex &) const;

  int rowCount(const QModelIndex &) const;

  QHash<int, QByteArray> roleNames() const;

public slots:
  QVariantHash getInputMapping(int index) const;
  int getInputDevicePort(int index) const;
  QString getInputDeviceName(int index) const;

private slots:
  void onInputInfoListRecieved(QList<InputDeviceInfo> inputInfoList);

private:
  QList<InputDeviceInfo> inputInfoCache;
  QHash<int, QByteArray> roles;
};
