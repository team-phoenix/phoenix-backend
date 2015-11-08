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
        Q_PROPERTY( bool television MEMBER television WRITE setTelevision NOTIFY televisionChanged )
        Q_PROPERTY( bool ntsc MEMBER ntsc WRITE setNtsc NOTIFY ntscChanged )
        Q_PROPERTY( bool widescreen MEMBER widescreen WRITE setWidescreen NOTIFY widescreenChanged )

    public:
        explicit VideoOutput( QQuickItem *parent = 0 );
        ~VideoOutput();

    signals:
        // Properties
        void aspectRatioChanged( qreal aspectRatio );
        void linearFilteringChanged( bool linearFiltering );
        void televisionChanged( bool television );
        void ntscChanged( bool ntsc );
        void widescreenChanged( bool widescreen );

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

        // The correct aspect ratio to display this picture in
        qreal aspectRatio;

        qreal calculateAspectRatio( ProducerFormat format );

        // Linear vs nearest-neighbor filtering
        bool linearFiltering;

        // Controls whether to use analog television mode or not (false = digital TV or handheld games, etc... anything
        // with a square PAR)
        bool television;

        // Do we expect 480 vertical lines (NTSC, true) or 576 vertical lines (PAL, false)?
        // Ignored if television is false
        bool ntsc;

        // Is this standard 4:3 (false) or is the image anamorphic widescreen (16:9 packed into a 4:3 frame, true)?
        // Ignored if television is false
        bool widescreen;

        // Setters for the above 3 properties, will force a recheck of the aspect ratio if any are called
        void setTelevision( bool television );
        void setNtsc( bool ntsc );
        void setWidescreen( bool widescreen );

        // Helper for printing aspect ratios as fractions
        // Source: http://codereview.stackexchange.com/questions/37189/euclids-algorithm-greatest-common-divisor
        int greatestCommonDivisor( int m, int n );
};

#endif // VIDEOOUTPUT_H
