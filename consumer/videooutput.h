#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H

#include "backendcommon.h"

#include "controllable.h"
#include "consumer.h"
#include "logging.h"

/*
 * VideoOutput is a consumer of video data as provided by Core via CoreControl. As it's a QML item, it will always live
 * in the QML thread.
 */

class VideoOutput : public QQuickItem, public Consumer, public Controllable {
        Q_OBJECT

        Q_PROPERTY( qreal aspectRatio MEMBER aspectRatio NOTIFY aspectRatioChanged )
        Q_PROPERTY( bool linearFiltering MEMBER linearFiltering NOTIFY linearFilteringChanged )

    public:
        explicit VideoOutput( QQuickItem *parent = 0 );
        ~VideoOutput();

    signals:
        // Properties
        void aspectRatioChanged( qreal aspectRatio );
        void linearFilteringChanged( bool linearFiltering );

        // A signal that can be hooked to drive frame production
        // Note that although it fires at vsync rate, it may not fire 100% of the time
        void windowUpdate();

    public slots:
        void consumerFormat( ProducerFormat consumerFmt ) override;
        void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) override;
        CONTROLLABLE_SLOT_SETSTATE_DEFAULT

    private:
        // The texture that will hold video frames from core. The texture itself lives in GPU RAM
        QSGTexture *texture;

        void pollStates();

        // Called from the scene graph (render) thread whenever it's time to redraw
        QSGNode *updatePaintNode( QSGNode *node, UpdatePaintNodeData *paintData ) override;

        qreal aspectRatio;
        bool linearFiltering;

        // Helper for printing aspect ratios as fractions
        // Source: http://codereview.stackexchange.com/questions/37189/euclids-algorithm-greatest-common-divisor
        int greatestCommonDivisor( int m, int n );
};

#endif // VIDEOOUTPUT_H
