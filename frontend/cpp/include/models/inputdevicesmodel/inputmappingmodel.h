#pragma once

#include "inputmappingentry.h"

#include <QAbstractListModel>
#include <QList>
#include <QVariantHash>

class InputMappingModel : public QAbstractListModel
{
  Q_OBJECT
public:

  enum Roles {
    MappingKey = Qt::UserRole + 1,
    MappingValue,
  };

  explicit InputMappingModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const;

  int columnCount(const QModelIndex &) const;

  int rowCount(const QModelIndex &) const;

  QHash<int, QByteArray> roleNames() const;

public slots:
  void setCurrentMapping(QVariantHash currentMapping);

private:
  QList<InputMappingEntry> inputMappingsCache;
  QHash<int, QByteArray> roles;
};
