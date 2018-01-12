#include <QString>
#include <QVariantHash>

struct System {
  int systemID{ -1 };
  QString systemName;
  QString systemShortName;

  System() = default;
  System(const QVariantHash &hash)
  {
    bool ok = false;
    systemID = hash.value("systemID").toInt(&ok);

    if (!ok) {
      systemID = -1;
    }

    systemName = hash.value("systemName").toString();
    systemShortName = hash.value("systemShortName").toString();
  }
};
