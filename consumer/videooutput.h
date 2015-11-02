#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H

#include "backendcommon.h"

#include "consumer.h"
#include "logging.h"

/*
 * VideoOutput is a consumer of video data as provided by Core via CoreControl. As it's a QML item, it will always live
 * in the QML thread.
 */

class VideoOutput : public QQuickItem, public Consumer {
        Q_OBJECT

        Q_PROPERTY( qreal aspectRatio MEMBER aspectRatio NOTIFY aspectRatioChanged )
        Q_PROPERTY( bool linearFiltering MEMBER linearFiltering NOTIFY linearFilteringChanged )

    public:
        explicit VideoOutput( QQuickItem *parent = 0 );
        ~VideoOutput();

    signals:
        void aspectRatioChanged( qreal aspectRatio );
        void linearFilteringChanged( bool linearFiltering );

    public slots:
        // Information about the type of data to expect
        void consumerFormat( ProducerFormat format ) override;

        // Must obtain a mutex to access the data. Only hold onto the mutex long enough to make a copy
        // Type can be one of the following: "audio", "video"
        void consumerData( QString type, QMutex *mutex, void *data, size_t bytes ) override;

    private:
        // The texture that will hold video frames from core. The texture itself lives in GPU RAM
        QSGTexture *texture;

        void pollStates();

        // Called from the scene graph (render) thread whenever it's time to redraw
        QSGNode *updatePaintNode( QSGNode *node, UpdatePaintNodeData *paintData ) override;

        qreal aspectRatio;
        bool linearFiltering;

};

#endif // VIDEOOUTPUT_H
