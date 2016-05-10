#pragma once

#include "node.h"
#include "libretrovariable.h"

class LibretroVariableForwarder : public Node
{
    Q_OBJECT
public:
    explicit LibretroVariableForwarder( QObject *parent = nullptr );

    virtual void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

signals:
    void variableFound( LibretroVariable var );


public slots:

};

