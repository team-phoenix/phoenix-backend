#pragma once

#include <QObject>

#include "videooutput.h"
#include "node.h"

// A wrapper for VideoOutput that enables it to exist as a node
// Necessary as VideoOutput, being a QQuickItem, cannot inherrit Node (not allowed in Qt)
class VideoOutputNode : public Node {
        Q_OBJECT
        Q_PROPERTY( VideoOutput *videoOutput MEMBER videoOutput NOTIFY videoOutputChanged )

    public:
        explicit VideoOutputNode( Node *parent = 0 );

    signals:
        void videoOutputChanged();

    public slots:
    private:
        VideoOutput *videoOutput;
};
