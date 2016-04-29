#pragma once

#include <QObject>

#include "node.h"
#include "phoenixwindow.h"

class PhoenixWindowNode : public Node {
        Q_OBJECT
        Q_PROPERTY( PhoenixWindow *phoenixWindow MEMBER phoenixWindow NOTIFY phoenixWindowChanged )

    public:
        explicit PhoenixWindowNode( Node *parent = 0 );
        PhoenixWindow *phoenixWindow{ nullptr };

    signals:
        void phoenixWindowChanged( PhoenixWindow *phoenixWindow );

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

        void frameSwapped();

    private:
        // Framerate of the monitor this Window exists in
        // TODO: Use whatever techniques are out there to get a more accurate number
        qreal hostFPS{ 60.0 };
};
