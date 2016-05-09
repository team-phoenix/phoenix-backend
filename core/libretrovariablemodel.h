#pragma once

#include "libretrovariable.h"

#include <QAbstractTableModel>
#include <QList>
#include <QHash>

class LibretroVariableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Role {
        Key = Qt::UserRole + 1,
        Value,
    };

    LibretroVariableModel( QObject *parent = nullptr );

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void appendVariable( const LibretroVariable &t_var );
    void removeVariable( LibretroVariable &t_var );

private:
    QList<LibretroVariable> m_varList;
};

