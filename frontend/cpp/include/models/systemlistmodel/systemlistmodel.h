#include "openvgdb.h"

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

  SystemModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex &index, int role) const;

  int rowCount(const QModelIndex &) const;

  QHash<int, QByteArray> roleNames() const
  {
    return roles;
  }

  void forceUpdate();

private:
  QList<System> systemCache;
  OpenVgDb openVgDb;
  QHash<int, QByteArray> roles;
};
