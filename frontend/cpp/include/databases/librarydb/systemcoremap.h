#pragma once

#include <QString>
#include <QVariantHash>

struct SystemCoreMap {
  QString systemFullName;
  QString coreName;
  bool isDefault{ false };

  QVariantHash getQueryFriendlyHash()
  {
    QVariantHash result;

    if (!systemFullName.isEmpty()) {
      result.insert("systemFullName", systemFullName);
    }

    if (!coreName.isEmpty()) {
      result.insert("coreName", coreName);
    }

    result.insert("isDefault", isDefault);

    return result;
  }
};
