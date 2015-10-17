#ifndef COREVIEW_H
#define COREVIEW_H

#include "backendcommon.h"

/*
 * VideoOutput is a consumer of video data as provided by GameManager.
 */

class VideoOutput : public QQuickItem {
        Q_OBJECT
    public:
        explicit VideoOutput( QQuickItem *parent = 0 );

    signals:

    public slots:
};

#endif // COREVIEW_H
