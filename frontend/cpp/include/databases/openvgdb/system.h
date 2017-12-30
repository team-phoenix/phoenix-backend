#include <QString>
#include <QVariantHash>

struct System {
  int systemID{ -1 };
  QString systemName;
  QString systemShortName;

  System() = default;
  System(const QVariantHash &hash)
  {
    systemID = hash.value("systemID").toInt();
    systemName = hash.value("systemName").toString();
    systemShortName = hash.value("systemShortName").toString();
  }
};
