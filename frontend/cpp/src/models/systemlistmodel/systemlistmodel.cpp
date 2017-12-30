#include "systemlistmodel.h"

SystemModel::SystemModel(QObject* parent)
  : QAbstractListModel(parent),
    roles{
  {
    {FullSystemName, "systemFullName"},
    {ShortSystemName, "systemShortName"},
  }
}
{
  forceUpdate();
}

QVariant SystemModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && index.row() < systemCache.size()) {
    const System &system = systemCache.at(index.row());

    switch (role) {
      case FullSystemName:
        return system.systemName;

      case ShortSystemName:
        return system.systemShortName;

      default:
        break;
    }
  }

  return QVariant();
}

int SystemModel::rowCount(const QModelIndex &) const
{
  return systemCache.size();
}

void SystemModel::forceUpdate()
{
  QList<System> systems = openVgDb.findAllSystems();

  beginRemoveRows(QModelIndex(), 0, systemCache.size());
  endRemoveRows();

  beginInsertRows(QModelIndex(), systemCache.size(), systemCache.size() + systems.size() - 1);
  systemCache = systems;
  endInsertRows();
}
