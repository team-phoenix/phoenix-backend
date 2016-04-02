#include "videooutput.h"

#include <QSGTexture>
#include <QSGSimpleTextureNode>
#include <QDateTime>
#include <QQuickWindow>
#include <QOpenGLContext>

VideoOutput::VideoOutput( QQuickItem *parent ) : QQuickItem( parent ), Consumer(), Controllable(),
    framebuffer( nullptr ),
    framebufferSize( 0 ),
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

    size_t newSize = consumerFmt.videoSize.width() * consumerFmt.videoSize.height() * consumerFmt.videoBytesPerPixel;
    framebuffer = new uchar[ newSize ]();
    framebufferSize = newSize;

}

VideoOutput::~VideoOutput() {
    if( framebuffer ) {
        delete framebuffer;
    }
}

void VideoOutput::consumerFormat( ProducerFormat format ) {

    qDebug() << "CONSUMER FORMAT START";

    // Update the property if the incoming format and related properties define a different aspect ratio than the one stored
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

    // Allocate a new framebuffer if the incoming format defines a larger one than we already have room for
    size_t newSize = format.videoSize.width() * format.videoSize.height() * format.videoBytesPerPixel;

    if( newSize > framebufferSize ) {
        if( framebuffer ) {
            delete framebuffer;
        }

        qCDebug( phxVideo ).nospace() << "Expanded framebuffer to fit new size/format (size = " <<
                                      ( double )newSize / 1024.0 << " KB)";
        framebuffer = new uchar[ newSize ]();
        framebufferSize = newSize;
    }


    qDebug() << "CONSUMER FORMAT END";

    consumerFmt = format;
}

void VideoOutput::consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) {
    Q_UNUSED( mutex )
    Q_UNUSED( bytes )
    Q_UNUSED( timestamp )

    // Copy framebuffer to our own buffer for later drawing
    if( type == QStringLiteral( "video" ) && currentState == Control::PLAYING ) {
        // Having this mutex active could mean a slow producer that holds onto the mutex for too long can block the UI
        // QMutexLocker locker( mutex );

        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

        // Discard data that's too far from the past to matter anymore
        if( currentTime - timestamp > 500 ) {
            static qint64 lastMessage = 0;

            if( currentTime - lastMessage > 1000 ) {
                lastMessage = currentTime;
                // qCWarning( phxVideo ) << "Discarding" << bytes << "bytes of old video data from" <<
                //                           currentTime - timestamp << "ms ago";
            }

            return;
        }

        const uchar *newFramebuffer = ( const uchar * )data;

        // Copy framebuffer line by line as the consumer may pack the image with arbitrary garbage data at the end of each line
        for( int i = 0; i < consumerFmt.videoSize.height(); i++ ) {
            // Don't read past the end of the given buffer
            Q_ASSERT( i * consumerFmt.videoBytesPerLine < bytes );

            // Don't write past the end of our framebuffer
            Q_ASSERT( i * consumerFmt.videoSize.width() * consumerFmt.videoBytesPerPixel < framebufferSize );

            memcpy( framebuffer + i * consumerFmt.videoSize.width() * consumerFmt.videoBytesPerPixel,
                    newFramebuffer + i * consumerFmt.videoBytesPerLine,
                    consumerFmt.videoSize.width() * consumerFmt.videoBytesPerPixel
                  );
        }

        // Schedule a call to updatePaintNode() for this Item
        update();
    }
}

QSGNode *VideoOutput::updatePaintNode( QSGNode *storedNode, QQuickItem::UpdatePaintNodeData *paintData ) {
    Q_UNUSED( paintData );

    // Let anyone who cares know that we've been asked for an update
    // FIXME: Remove this, hook custom render loop instead or some other vsync'd source
    emit windowUpdate( QDateTime::currentMSecsSinceEpoch() );

    // Don't draw unless emulation is active
    if( currentState != Control::PLAYING && currentState != Control::PAUSED ) {
        return 0;
    }

    // Create a new texture node for the renderer if one does not already exist
    QSGSimpleTextureNode *storedTextureNode = static_cast<QSGSimpleTextureNode *>( storedNode );

    if( !storedTextureNode ) {
        storedTextureNode = new QSGSimpleTextureNode();
    }

    // Schedule old texture for deletion
    if( texture ) {
        texture->deleteLater();
        texture = nullptr;
    }

    // Create new Image that holds a reference to our framebuffer
    QImage image( ( const uchar * )framebuffer, consumerFmt.videoSize.width(), consumerFmt.videoSize.height(),
                  consumerFmt.videoPixelFormat );

    // Create a texture via a factory function (framebuffer contents are uploaded to GPU once QSG reads texture node)
    texture = window()->createTextureFromImage( image, QQuickWindow::TextureOwnsGLTexture );

    // Ensure texture lives in rendering thread so it will be deleted only once it's no longer associated with
    // the texture node
    texture->moveToThread( window()->openglContext()->thread() );

    // Put this new texture into our QSG node and mark the node dirty so it'll be redrawn
    storedTextureNode->setTexture( texture );
    storedTextureNode->setRect( boundingRect() );
    storedTextureNode->setFiltering( linearFiltering ? QSGTexture::Linear : QSGTexture::Nearest );

    storedTextureNode->markDirty( QSGNode::DirtyMaterial );

    return storedTextureNode;
}

qreal VideoOutput::calculateAspectRatio( ProducerFormat format ) {
    qreal newRatio = format.videoAspectRatio;

    if( widescreen ) {
        newRatio = 16.0 / 9.0;
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

