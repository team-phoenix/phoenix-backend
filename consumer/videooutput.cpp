#include "videooutput.h"

VideoOutput::VideoOutput( QQuickItem *parent ) : QQuickItem( parent ),
    texture( nullptr ),
    aspectRatio( 1.0 ),
    linearFiltering( false ),
    television( false ),
    ntsc( true ),
    widescreen( false ) {

    // Mandatory for our own drawing code to do anything
    setFlag( QQuickItem::ItemHasContents, true );

    emit aspectRatioChanged( aspectRatio );
    emit linearFilteringChanged( linearFiltering );
    emit televisionChanged( television );
    emit ntscChanged( ntsc );
    emit widescreenChanged( widescreen );

}

VideoOutput::~VideoOutput() {

}

void VideoOutput::consumerFormat( ProducerFormat format ) {

    qreal newRatio = calculateAspectRatio( format );

    if( aspectRatio != newRatio || format.videoSize.width() != consumerFmt.videoSize.width() || format.videoSize.height() != consumerFmt.videoSize.height() ) {

        // Pretty-print the old and new aspect ratio (ex. "4:3")
        int oldAspectRatioX = consumerFmt.videoSize.height() * aspectRatio;
        int oldAspectRatioY = consumerFmt.videoSize.width() / aspectRatio;
        int newAspectRatioX = format.videoSize.height() * newRatio;
        int newAspectRatioY = format.videoSize.width() / newRatio;
        int oldGCD = greatestCommonDivisor( oldAspectRatioX, oldAspectRatioY );
        int newGCD = greatestCommonDivisor( newAspectRatioX, newAspectRatioY );
        int oldAspectRatioXInt = oldAspectRatioX / oldGCD;
        int oldAspectRatioYInt = oldAspectRatioY / oldGCD;
        int newAspectRatioXInt = newAspectRatioX / newGCD;
        int newAspectRatioYInt = newAspectRatioY / newGCD;
        qCDebug( phxVideo ).nospace() << "Aspect ratio changed to " << newAspectRatioXInt << ":" << newAspectRatioYInt
                                      << " (was " << oldAspectRatioXInt << ":" << oldAspectRatioYInt << ")" ;

        aspectRatio = newRatio;
        emit aspectRatioChanged( aspectRatio );

    }

    consumerFmt = format;

}

void VideoOutput::consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) {
    // Buffer pool makes this unnecessary
    Q_UNUSED( mutex )

    Q_UNUSED( bytes )
    Q_UNUSED( timestamp )

    if( type == QStringLiteral( "video" ) && currentState == Control::PLAYING ) {

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

    emit windowUpdate();

    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>( node );

    if( !textureNode ) {
        textureNode = new QSGSimpleTextureNode();
    }

    // Do not use the texture if it doesn't exist or the buffer passed down the line is no longer valid
    // FIXME: A race condition involving the buffer passed from ConsumerData down to this texture might still be possible
    // if the producer shuts down fast enough and the buffer is freed before the QSG thread can use it, crashes can
    // still happen
    if( !texture || ( currentState != Control::PLAYING && currentState != Control::PAUSED ) ) {
        return QQuickItem::updatePaintNode( textureNode, paintData );
    }

    textureNode->setTexture( texture );
    textureNode->setRect( boundingRect() );

    if( linearFiltering ) {
        textureNode->setFiltering( QSGTexture::Linear );
    } else {
        textureNode->setFiltering( QSGTexture::Nearest );
    }

    return textureNode;
}

qreal VideoOutput::calculateAspectRatio( ProducerFormat format ) {
    qreal newRatio = ( double )format.videoSize.width() / format.videoSize.height();

    if( television ) {

        // We always assume that the given framebuffer was meant to be stretched across the entire scanline
        // Thus, we don't touch the numerator
        qreal parNumerator = 1.0;
        qreal parDenominator = 1.0;
        qreal inputAspectRatio = 1.0;

        if( ntsc ) {
            parDenominator = ( format.videoSize.height() > 240 ) ?
                             ( double )format.videoSize.height() / 480.0 :
                             ( double )format.videoSize.height() / 240.0 ;
        }

        // PAL
        else {
            parDenominator = ( format.videoSize.height() > 288 ) ?
                             ( double )format.videoSize.height() / 576.0 :
                             ( double )format.videoSize.height() / 288.0 ;
        }

        // Anamorphic widescreen
        if( widescreen ) {
            inputAspectRatio = 16.0 / 9.0;
        }

        // Non-widescreen
        else {
            inputAspectRatio = 4.0 / 3.0;
        }

        // Divide the intended output aspect ratio by the PAR to get the final ratio
        newRatio = inputAspectRatio / ( parNumerator / parDenominator );

    }

    return newRatio;
}

void VideoOutput::setTelevision( bool television ) {
    if( this->television != television ) {
        this->television = television;
        consumerFormat( consumerFmt );
    }
}

void VideoOutput::setNtsc( bool ntsc ) {
    if( this->ntsc != ntsc ) {
        this->ntsc = ntsc;
        consumerFormat( consumerFmt );
    }
}

void VideoOutput::setWidescreen( bool widescreen ) {
    if( this->widescreen != widescreen ) {
        this->widescreen = widescreen;
        consumerFormat( consumerFmt );
    }
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

