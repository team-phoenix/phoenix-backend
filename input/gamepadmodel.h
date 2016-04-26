#pragma once

#include <QAbstractTableModel>

class Gamepad;
class QQmlEngine;
class QJSEngine;

class GamepadModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Role {
        Name = Qt::UserRole + 1,
        GamepadRole,
        Type,
        Mapping,
    };

    explicit GamepadModel( QObject *parent = nullptr );
    ~GamepadModel();

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    static QObject *registerSingletonCallback( QQmlEngine *engine, QJSEngine *jsEngine );

public slots:
    void addGamepad( const Gamepad *t_gamepad );
    void removeGamepad( const Gamepad *t_gamepad );

signals:

private:
    QHash<int, QByteArray> m_roleNames;
    QList<const Gamepad *> m_gamepadList;
    Gamepad *m_keyboard;

};
