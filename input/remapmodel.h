#pragma once

#include <QAbstractListModel>

class QQmlEngine;
class QJSEngine;
class Gamepad;

class RemapModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum Roles {
        ButtonKey = Qt::UserRole + 1,
        ButtonValue,
    };

    explicit RemapModel( QObject *parent = nullptr );

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    static QObject *registerSingletonCallback( QQmlEngine *engine, QJSEngine *jsEngine );

    Q_INVOKABLE void addGamepad( const Gamepad *t_gamepad );
    Q_INVOKABLE void removeGamepad( const Gamepad *t_gamepad );
    Q_INVOKABLE void currentIndex( int _index );

private:
    int m_index{ -1 };
    QHash<int, QByteArray> m_roleNames;

    QList<const Gamepad *> m_gamepadList;

    QStringList m_gamepadKeys;
    QStringList m_gamepadValues;
};

