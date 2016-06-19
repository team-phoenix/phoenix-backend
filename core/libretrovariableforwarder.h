#pragma once

#include "pipeline.h"
#include "libretrovariable.h"

class LibretroVariableForwarder : public Node {
        Q_OBJECT

    public:
        explicit LibretroVariableForwarder( QObject *parent = nullptr );

        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

        void connectDependencies(QMap<QString, QObject *> objects ) override;
        void disconnectDependencies( QMap<QString, QObject *> objects ) override;

    signals:
        void insertVariable( QString key, QStringList values, QString currentValue, QString description );
        void clearVariables();

    public slots:
        void setVariable( QString key, QString value );
};
