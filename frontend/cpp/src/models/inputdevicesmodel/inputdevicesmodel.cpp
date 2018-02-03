#include "inputdevicesmodel.h"
#include "emulationlistener.h"

InputDeviceInfoModel::InputDeviceInfoModel(QObject* parent)
  : QAbstractListModel(parent),
    roles {
  { InputDeviceName, "inputDeviceName"},
  { InputDevicePort, "inputDevicePort"},
}
{
  connect(&EmulationListener::instance(), &EmulationListener::inputInfoListRecieved, this,
          &InputDeviceInfoModel::onInputInfoListRecieved);
}

QVariant InputDeviceInfoModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && index.row() < inputInfoCache.size()) {

    const InputDeviceInfo &deviceInfo = inputInfoCache.at(index.row());

    switch (role) {

      case InputDevicePort:
        return deviceInfo.inputDevicePort;

      case InputDeviceName:
        return deviceInfo.inputDeviceName;

      default:
        break;
    }
  }

  return QVariant();
}

int InputDeviceInfoModel::columnCount(const QModelIndex &) const
{
  return roles.size();
}

int InputDeviceInfoModel::rowCount(const QModelIndex &) const
{
  return inputInfoCache.size();
}

QHash<int, QByteArray> InputDeviceInfoModel::roleNames() const
{
  return roles;
}

void InputDeviceInfoModel::onInputInfoListRecieved(QList<InputDeviceInfo> inputInfoList)
{
  if (!inputInfoList.isEmpty()) {
    beginInsertRows(QModelIndex(), inputInfoCache.size(), inputInfoList.size() - 1);
    inputInfoCache = inputInfoList;
    endInsertRows();
  }
}
