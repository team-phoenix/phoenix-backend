#include "videooutput.h"

VideoOutput::VideoOutput( QQuickItem *parent ) : QQuickItem( parent ),
    texture( nullptr ),
    aspectRatio( 1.0 ),
    linearFiltering( false ) {

    // Mandatory for our own drawing code to do anything
    setFlag( QQuickItem::ItemHasContents, true );

    update();
}

VideoOutput::~VideoOutput() {

}

void VideoOutput::consumerFormat( ProducerFormat format ) {

    int oldGCD = greatestCommonDivisor( consumerFmt.videoSize.width(), consumerFmt.videoSize.height() );
    int newGCD = greatestCommonDivisor( format.videoSize.width(), format.videoSize.height() );
    int oldAspectRatioX = consumerFmt.videoSize.width() / oldGCD;
    int oldAspectRatioY = consumerFmt.videoSize.height() / oldGCD;
    int newAspectRatioX = format.videoSize.width() / newGCD;
    int newAspectRatioY = format.videoSize.height() / newGCD;

    consumerFmt = format;

    qreal newRatio = ( double )format.videoSize.width() / format.videoSize.height();

    if( aspectRatio != newRatio ) {
        aspectRatio = newRatio;
        emit aspectRatioChanged( aspectRatio );
        qCDebug( phxVideo ).nospace() << "Aspect ratio changed to " << newAspectRatioX << ":" << newAspectRatioY
                                      << " (was " << oldAspectRatioX << ":" << oldAspectRatioY << ")" ;
    }

    update();

}

void VideoOutput::consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) {
    Q_UNUSED( bytes )
    Q_UNUSED( timestamp )

    if( type == QStringLiteral( "video" ) ) {
        QMutexLocker lock( mutex );

        // Schedule old texture for deletion
        if( texture ) {
            texture->deleteLater();
            texture = nullptr;
        }

        // Create new image (data is convered to an integer array, stored in heap until end of this block)
        QImage image = QImage( ( const uchar * )data, consumerFmt.videoSize.width(), consumerFmt.videoSize.height(),
                               consumerFmt.videoBytesPerLine, consumerFmt.videoPixelFormat );

        // Create a texture (data is uploaded to GPU)
        texture = window()->createTextureFromImage( image, QQuickWindow::TextureOwnsGLTexture );

        // Ensure texture lives in rendering thread so it will be deleted at the appropiate time
        texture->moveToThread( window()->openglContext()->thread() );

        update();
    }

}

QSGNode *VideoOutput::updatePaintNode( QSGNode *node, QQuickItem::UpdatePaintNodeData *paintData ) {
    Q_UNUSED( paintData );

    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>( node );

    if( !textureNode ) {
        textureNode = new QSGSimpleTextureNode();
    }

    // First frame, no video data yet. Tell core to render a frame
    // then display it next time.
    if( !texture ) {
        return QQuickItem::updatePaintNode( textureNode, paintData );
    }

    textureNode->setTexture( texture );
    textureNode->setRect( boundingRect() );

    if( linearFiltering ) {
        textureNode->setFiltering( QSGTexture::Linear );
    } else {
        textureNode->setFiltering( QSGTexture::Nearest );
    }

    textureNode->setTextureCoordinatesTransform( QSGSimpleTextureNode::MirrorVertically |
            QSGSimpleTextureNode::MirrorHorizontally );

    emit windowUpdate();

    return textureNode;
}

int VideoOutput::greatestCommonDivisor( int m, int n ) {
    int r;

    /* Check For Proper Input */
    if( ( m == 0 ) || ( n == 0 ) ) {
        return 0;
    } else if( ( m < 0 ) || ( n < 0 ) ) {
        return -1;
    }

    do {
        r = m % n;

        if( r == 0 ) {
            break;
        }

        m = n;
        n = r;
    } while( true );

    return n;
}

