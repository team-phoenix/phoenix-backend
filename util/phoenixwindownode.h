#pragma once

#include <QObject>
#include <QRect>

#include "node.h"
#include "phoenixwindow.h"

/*
 * PhoenixWindowNode is a Node that wraps around an underlying PhoenixWindow. It fires heartbeat commands whenever the
 * window emits a QQuickWindow::frameSwapped() signal and will tell the underlying window whenever VSync mode changes.
 */

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
        void geometryChanged();

    private:
        // Framerate of the monitor this Window exists in
        // TODO: Use whatever techniques are out there to get a more accurate number
        qreal hostFPS{ 60.0 };

        // Window geometry
        QRect geometry;
};
