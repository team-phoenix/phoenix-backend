#include "inputdevicesmodel.h"
#include "emulationlistener.h"

InputDeviceInfoModel::InputDeviceInfoModel(QObject* parent)
  : QAbstractListModel(parent),
    roles {
  { InputDeviceName, "inputDeviceName"},
  { InputDevicePort, "inputDevicePort"},
  { InputDeviceDisplayName, "inputDeviceDisplayName"},
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

      case InputDeviceDisplayName:
        return QString("%1 %2").arg(deviceInfo.inputDeviceName,
                                    QString::number(deviceInfo.inputDevicePort));

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

QVariantHash InputDeviceInfoModel::getInputMapping(int index) const
{
  return inputInfoCache.at(index).inputMapping;
}

int InputDeviceInfoModel::getInputDevicePort(int index) const
{
  return inputInfoCache.at(index).inputDevicePort;
}

QString InputDeviceInfoModel::getInputDeviceName(int index) const
{
  return inputInfoCache.at(index).inputDeviceName;
}

void InputDeviceInfoModel::onInputInfoListRecieved(QList<InputDeviceInfo> inputInfoList)
{
  if (!inputInfoList.isEmpty()) {
    beginInsertRows(QModelIndex(), inputInfoCache.size(), inputInfoList.size() - 1);
    inputInfoCache = inputInfoList;
    endInsertRows();
  }
}
