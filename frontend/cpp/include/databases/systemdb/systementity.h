#pragma once

#include <QString>
#include <QVariantHash>

struct SystemEntity {
  QString systemFullName;
  QString systemShortName;

  SystemEntity(QVariantHash hash)
  {
    systemFullName = hash.value("systemFullName").toString();
    systemShortName = hash.value("systemShortName").toString();
  }

  SystemEntity() = default;

  QVariantHash getHashedRowValues()
  {
    return {
      { "systemFullName", systemFullName},
      { "systemShortName", systemShortName},
    };
  }
};
