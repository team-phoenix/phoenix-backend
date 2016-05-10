#pragma once

#include "libretrovariable.h"

#include <QAbstractTableModel>
#include <QList>
#include <QHash>

class LibretroVariableForwarder;

class LibretroVariableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Role {
        Key = Qt::UserRole + 1,
        Choices,
        Description,
    };

    LibretroVariableModel( QObject *parent = nullptr );

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    void setForwarder( LibretroVariableForwarder *t_forwarder );

public slots:
    void appendVariable( LibretroVariable t_var );
    void updateVariable( QString t_key, QString t_value );

private slots:
    void clear();

private:
    QList<LibretroVariable> m_varList;
    LibretroVariableForwarder *m_forwarder{ nullptr };

    QHash<int, QByteArray> m_roleNames;
};
