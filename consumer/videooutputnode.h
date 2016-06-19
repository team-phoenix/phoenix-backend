#pragma once

#include <QObject>

#include "videooutput.h"
#include "pipeline.h"

// A wrapper for VideoOutput that enables it to exist as a node
// Necessary as VideoOutput, being a QQuickItem, cannot inherrit Node (not allowed in Qt)
// TODO: Safety checks?
class VideoOutputNode : public Node {
        Q_OBJECT

    public:
        explicit VideoOutputNode( Node *parent = nullptr );
        void connectDependencies( QMap<QString, QObject *> objects ) override;
        void disconnectDependencies( QMap<QString, QObject *> objects ) override;

    signals:

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;
        void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) override;

    private:
        VideoOutput *videoOutput{ nullptr };
};
