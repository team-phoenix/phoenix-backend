#pragma once

#include <QObject>

class Axes : public QObject {
    Q_OBJECT
    Q_PROPERTY( qint16 a READ a NOTIFY aChanged)
    Q_PROPERTY( qint16 b READ b NOTIFY bChanged)
    Q_PROPERTY( qint16 x READ x NOTIFY xChanged)
    Q_PROPERTY( qint16 y READ y NOTIFY yChanged)
public:
    explicit Axes( QObject *parent = nullptr )
        : QObject( parent ) {

    }

    qint16 a() const {
        return m_a;
    }

    qint16 b() const {
        return m_b;
    }

    qint16 x() const
    {
        return m_x;
    }

    qint16 y() const {
        return m_y;
    }

signals:
    void aChanged();
    void bChanged();
    void xChanged();
    void yChanged();

private:
    qint16 m_a{ 0 };
    qint16 m_b{ 0 };
    qint16 m_x{ 0 };
    qint16 m_y{ 0 };

    void setA( qint16 t_state ) {
        m_a = t_state ;
        emit aChanged();
    }

    void setB( qint16 t_state  ) {
        m_b = t_state ;
        emit bChanged();
    }

    void setX( qint16 t_state  ) {
        m_x = t_state ;
        emit xChanged();
    }

    void setY( qint16 t_state  ) {
        m_y = t_state ;
        emit yChanged();
    }

};


class Buttons : public QObject {
    Q_OBJECT
public:
    explicit Buttons( QObject *parent = nullptr )
        : QObject( parent ) {

    }

};

class GlobalGamepad : public QObject
{
    Q_OBJECT
    Q_PROPERTY( Buttons * buttons READ buttons )
    Q_PROPERTY( Axes * axes READ axes )

public:
    explicit GlobalGamepad(QObject *parent = nullptr);
    ~GlobalGamepad() = default;

    Buttons *buttons() const {
        return &m_buttons;
    }

    Axes *axes() const {
        return &m_axes;
    }

private:
    mutable Buttons m_buttons;
    mutable Axes m_axes;


};
