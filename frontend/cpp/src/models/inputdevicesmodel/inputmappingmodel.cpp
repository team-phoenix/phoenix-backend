#include "inputmappingmodel.h"

#include <QDebug>

InputMappingModel::InputMappingModel(QObject* parent)
  : QAbstractListModel(parent),
    roles {
  { MappingKey, "mappingKey"},
  { MappingValue, "mappingValue"},
}
{

}

QVariant InputMappingModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && index.row() < inputMappingsCache.size()) {

    const InputMappingEntry &mappingEntry = inputMappingsCache.at(index.row());

    switch (role) {

      case MappingKey:
        return mappingEntry.mappingKey;

      case MappingValue:
        return mappingEntry.mappingValue;

      default:
        break;
    }
  }

  return QVariant();
}

int InputMappingModel::columnCount(const QModelIndex &) const
{
  return roles.size();
}

int InputMappingModel::rowCount(const QModelIndex &) const
{
  return inputMappingsCache.size();
}

QHash<int, QByteArray> InputMappingModel::roleNames() const
{
  return roles;
}

void InputMappingModel::setCurrentMapping(QVariantHash currentMapping)
{
  if (!inputMappingsCache.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, inputMappingsCache.size());
    endRemoveRows();
  }

  beginInsertRows(QModelIndex(), 0, currentMapping.size() - 1);

  for (auto iter = currentMapping.begin(); iter != currentMapping.end(); ++iter) {
    inputMappingsCache.append(InputMappingEntry(iter.key(), iter.value().toString()));
  }

  endInsertRows();
}
