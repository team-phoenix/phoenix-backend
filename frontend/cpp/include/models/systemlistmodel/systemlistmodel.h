#pragma once

#include "systemdb.h"

#include <QAbstractListModel>
#include <QList>

class SystemModel : public QAbstractListModel
{
  Q_OBJECT
public:

  enum Roles {
    FullSystemName = Qt::UserRole + 1,
    ShortSystemName,
  };

  explicit SystemModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const;

  int rowCount(const QModelIndex &) const;

  QHash<int, QByteArray> roleNames() const;

  void forceUpdate();

private:
  QList<SystemEntity> systemCache;
  SystemDb systemDb;
  QHash<int, QByteArray> roles;
};
