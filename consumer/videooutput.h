#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H

#include "pipelinenode.h"
#include "controllable.h"
#include "consumer.h"
#include "logging.h"

#include <QQuickItem>

/*
 * VideoOutput is a consumer of video data as provided by Core via CoreControl. As it's a QML item, it will always live
 * in the QML thread.
 */

class QSGTexture;

class VideoOutput : public QQuickItem, public Consumer, public Controllable {
        Q_OBJECT
        PHX_PIPELINE_INTERFACE( VideoOutput )

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
        void windowUpdate( qint64 timestamp );

    public slots:
        void consumerFormat( ProducerFormat consumerFmt ) override;
        void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) override;
        CONTROLLABLE_SLOT_SETSTATE_DEFAULT

        void dataIn( PipelineNode::DataReason t_reason
                     , QMutex *t_mutex
                     , void *t_data
                     , size_t t_bytes
                     , qint64 t_timeStamp ) {
            switch( t_reason ) {
                case PipelineNode::State_Changed_To_Loading_bool:
                case PipelineNode::State_Changed_To_Paused_bool:
                case PipelineNode::State_Changed_To_Playing_bool:
                case PipelineNode::State_Changed_To_Stopped_bool:
                case PipelineNode::State_Changed_To_Unloading_bool:
                    setNodeState( t_reason );
                    break;

                case PipelineNode::Format_Changed_ProducerFormat: {
                    consumerFormat( *static_cast<ProducerFormat *>( t_data ) );
                    break;
                }
                case PipelineNode::Create_Frame_any: {

                    if ( nodeState() == PipelineNode::State_Changed_To_Playing_bool ) {
                        updateFrame( t_mutex, t_data, t_bytes, t_timeStamp );
                    }
                    break;
                }
                default:
                    break;
            }

            emitDataOut( t_reason, t_data, t_bytes, t_timeStamp );
        }

    private:
        // The framebuffer that holds the latest frame from Core
        uchar *framebuffer;
        size_t framebufferSize;

        // Holds a pointer to the framebuffer via its underlying QImage, used by renderer to upload framebuffer to GPU
        QSGTexture *texture;

        // Called by render thread whenever it's time to render and needs an update from us
        // We'll assign the current texture to the stored node given to us, creating this stored node if it does not
        // already exist
        // Main thread (our thread) is guarantied to be suspended while this function executes, the rest run normally
        // paintData is a pointer to a QSGTransformNode which contains the transformation matrix (unused)
        QSGNode *updatePaintNode( QSGNode *storedNode, UpdatePaintNodeData *paintData ) override;

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

        void updateFrame( QMutex *t_mutex, void *t_data, size_t t_bytes, qint64 t_timestamp );
};

Q_DECLARE_METATYPE( VideoOutput * )

#endif // VIDEOOUTPUT_H
