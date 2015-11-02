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
    this->format = format;

    this->aspectRatio = ( double )format.videoSize.width() / format.videoSize.height();
    emit aspectRatioChanged( aspectRatio );

    update();
}

void VideoOutput::consumerData( QString type, QMutex *mutex, void *data, size_t bytes ) {
    Q_UNUSED( bytes )

    if( type == QStringLiteral( "video" ) ) {
        QMutexLocker lock( mutex );

        // Schedule old texture for deletion
        if( texture ) {
            texture->deleteLater();
            texture = nullptr;
        }

        // Create new image (data is convered to an integer array, stored in heap until end of this block)
        QImage image = QImage( ( const uchar * )data, format.videoSize.width(), format.videoSize.height(),
                               format.videoBytesPerLine, format.videoPixelFormat );

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

    return textureNode;
}
