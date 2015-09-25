#include "videoitem.h"

VideoItem::VideoItem( QQuickItem *parent ) :
    QQuickItem( parent ),
    qmlRunning( false ),
    qmlInputManager( nullptr ),
    audioOutput( new AudioOutput() ), audioOutputThread( new QThread( this ) ),
    core( new Core() ), // coreTimer( new QTimer() ),
    coreThread( nullptr ), qmlCoreState( Core::STATEUNINITIALIZED ),
    avInfo(), pixelFormat(),
    corePath( "" ), gamePath( "" ),
    width( 0 ), height( 0 ), pitch( 0 ), coreFPS( 0.0 ), hostFPS( 0.0 ), aspectRatio( 1.0 ),
    texture( nullptr ),
    frameTimer() {

    setFlag( QQuickItem::ItemHasContents, true );

    // Connect controller signals and slots

    // Run a timer to make core produce a frame at regular intervals, or at vsync
    // coreTimer disabled at the moment due to the granulatiry being 1ms (not good enough)
    // connect( &coreTimer, &QTimer::timeout, &core, &Core::slotFrame );
    connect( this, &VideoItem::signalFrame, core, &Core::slotFrame );

    // Do the next item in the core lifecycle when the state has changed
    connect( core, &Core::signalCoreStateChanged, this, &VideoItem::slotCoreStateChanged );

    // Load a core and a game
    connect( this, &VideoItem::signalLoadCore, core, &Core::slotLoadCore );
    connect( this, &VideoItem::signalLoadGame, core, &Core::slotLoadGame );

    // Get the audio and video timing/format from core once a core and game are loaded,
    // send the data out to each consumer for their own initialization
    connect( core, &Core::signalAVFormat, this, &VideoItem::slotCoreAVFormat );
    connect( this, &VideoItem::signalAudioFormat, audioOutput, &AudioOutput::slotAudioFormat );
    connect( this, &VideoItem::signalVideoFormat, this, &VideoItem::slotVideoFormat ); // Belongs in both categories

    // Do the next item in the core lifecycle when its state changes
    connect( this, &VideoItem::signalRunChanged, audioOutput, &AudioOutput::slotSetAudioActive );

    // Connect consumer signals and slots

    connect( core, &Core::signalAudioData, audioOutput, &AudioOutput::slotAudioData );
    connect( core, &Core::signalVideoData, this, &VideoItem::slotVideoData );

    // Set up threads

    // Place the objects under VideoItem's control into their own threads
    audioOutput->moveToThread( audioOutputThread );

    // Ensure the objects are cleaned up when it's time to quit and destroyed once their thread is done
    // Also, ensure their cleanup is blocking. We DON'T want anything else happening while cleanup is being done
    // Consumers go first. Buffer pool should not be cleared until the consumers stop consuming
    connect( this, &VideoItem::signalShutdown, audioOutput, &AudioOutput::slotShutdown, Qt::BlockingQueuedConnection );
    connect( this, &VideoItem::signalShutdown, core, &Core::slotShutdown/*, Qt::BlockingQueuedConnection*/ );
    connect( audioOutputThread, &QThread::finished, audioOutput, &AudioOutput::deleteLater );

    // Catch the user exit signal and clean up
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [ = ]() {

        qCDebug( phxController ) << "===========QCoreApplication::aboutToQuit()===========";

        // Shut down Core and the consumers
        if( coreState() != Core::STATEUNINITIALIZED ) {
            emit signalShutdown();
        }

        // Stop processing events in the other threads, then block the main thread until they're finished

        // Stop consumer threads
        audioOutputThread->exit();
        audioOutputThread->wait();
        audioOutputThread->deleteLater();

    } );

    // Start threads

    audioOutputThread->start();

}

VideoItem::~VideoItem() {

}

//
// Controller methods
//

InputManager *VideoItem::inputManager() const {
    return qmlInputManager;
}

void VideoItem::setInputManager( InputManager *manager ) {

    if( manager != qmlInputManager ) {

        qmlInputManager = manager;
        core->inputManager = qmlInputManager;

        connect( this, &VideoItem::signalRunChanged, qmlInputManager, &InputManager::setRun, Qt::DirectConnection );

        emit inputManagerChanged();

    }

}

void VideoItem::registerTypes() {
    qmlRegisterType<VideoItem>( "vg.phoenix.backend", 1, 0, "VideoItem" );
    qmlRegisterType<Core>( "vg.phoenix.backend", 1, 0, "Core" );

    // Don't let the Qt police find out we're declaring these structs as metatypes
    // without proper constructors/destructors declared/written
    qRegisterMetaType<retro_system_av_info>();
    qRegisterMetaType<retro_pixel_format>();
    qRegisterMetaType<Core::State>();
    qRegisterMetaType<Core::Error>();
}

bool VideoItem::running() const {
    return qmlRunning;
}

Core::State VideoItem::coreState() const {
    return qmlCoreState;
}

void VideoItem::setRunning( const bool running ) {
    if( qmlRunning != running ) {
        qmlRunning = running;
        emit signalRunChanged( running );
    }
}

void VideoItem::slotCoreStateChanged( Core::State newState, Core::Error error ) {

    qCDebug( phxController ) << "slotStateChanged(" << Core::stateToText( newState ) << "," << error << ")";

    setCoreState( newState );

    switch( newState ) {

        case Core::STATEUNINITIALIZED:
            break;

        // Time to run the game
        case Core::STATEREADY:

            // This is mixing control (coreThread) and consumer (render thread) members...

            if( !coreThread ) {
                coreThread = window()->openglContext()->thread();

                // Place Core into the render thread
                // Mandatory for OpenGL cores
                // Also prevents massive overhead/performance loss caused by QML effects (like FastBlur)
                core->moveToThread( coreThread );
                connect( coreThread, &QThread::finished, core, &Core::deleteLater );

                // Disconnect and reconnect core-related signals now that VideoItem and Core share the same thread
                disconnect( core, &Core::signalCoreStateChanged, this, &VideoItem::slotCoreStateChanged );
                connect( core, &Core::signalCoreStateChanged, this, &VideoItem::slotCoreStateChanged );

                disconnect( this, &VideoItem::signalLoadCore, core, &Core::slotLoadCore );
                disconnect( this, &VideoItem::signalLoadGame, core, &Core::slotLoadGame );
                connect( this, &VideoItem::signalLoadCore, core, &Core::slotLoadCore );
                connect( this, &VideoItem::signalLoadGame, core, &Core::slotLoadGame );

                qCDebug( phxController ) << "Begin emulation.";

                // Let all the consumers know emulation began
                emit signalRunChanged( true );

            }

            setRunning( true );

            // Get core to immediately (sorta) produce the first frame
            emit signalFrame();

            // Force an update to keep the render thread from pausing
            update();

            break;

        case Core::STATEPAUSED:
            setRunning( false );
            break;

        case Core::STATEFINISHED:
            setRunning( false );
            emit signalShutdown();
            break;

        case Core::STATEERROR:
            switch( error ) {
                default:
                    break;
            }

            break;

        default:
            break;
    }

}

void VideoItem::slotCoreAVFormat( retro_system_av_info avInfo, retro_pixel_format pixelFormat ) {

    this->avInfo = avInfo;
    this->pixelFormat = pixelFormat;

    // TODO: Set this properly, either with testing and averages (RA style) or via EDID (proposed)
    double monitorRefreshRate = 60.0;

    emit signalAudioFormat( avInfo.timing.sample_rate, avInfo.timing.fps, monitorRefreshRate );
    emit signalVideoFormat( pixelFormat,
                            avInfo.geometry.max_width,
                            avInfo.geometry.max_height,
                            avInfo.geometry.max_width * ( pixelFormat == RETRO_PIXEL_FORMAT_XRGB8888 ? 4 : 2 ),
                            avInfo.timing.fps, monitorRefreshRate );

}

void VideoItem::slotPause() {
    if( coreState() != Core::STATEPAUSED ) {
        slotCoreStateChanged( Core::STATEPAUSED, Core::CORENOERROR );
    }
}

void VideoItem::slotResume() {
    if( coreState() != Core::STATEREADY ) {
        slotCoreStateChanged( Core::STATEREADY, Core::CORENOERROR );
    }
}

void VideoItem::slotStop() {
    if( coreState() != Core::STATEUNINITIALIZED ) {
        slotCoreStateChanged( Core::STATEFINISHED, Core::CORENOERROR );
    }

}

void VideoItem::setCore( QString libretroCore ) {

    // Do nothing if a blank string is given
    if( libretroCore.isEmpty() ) {
        return;
    }

    // Stop the game if it's currently running
    if( coreState() != Core::STATEUNINITIALIZED ) {
        qCDebug( phxController ) << "Stopping currently running game:" << gamePath;
        slotCoreStateChanged( Core::STATEFINISHED, Core::CORENOERROR );
    }

    corePath = libretroCore;
    emit signalLoadCore( corePath );

}

void VideoItem::setGame( QString game ) {

    // Do nothing if a blank string is given
    if( game.isEmpty() ) {
        return;
    }

    gamePath = game;
    emit signalLoadGame( gamePath );

}

//
// Consumer methods
//

void VideoItem::slotVideoFormat( retro_pixel_format pixelFormat, int width, int height, int pitch,
                                 double coreFPS, double hostFPS ) {

    qCDebug( phxVideo() ) << "pixelformat =" << pixelFormat << "width =" << width << "height =" << height
                          << "pitch =" << pitch << "coreFPS =" << coreFPS << "hostFPS =" << hostFPS;

    this->pixelFormat = pixelFormat;
    this->width = width;
    this->height = height;
    this->pitch = pitch;
    this->coreFPS = coreFPS;
    this->hostFPS = hostFPS;
    this->aspectRatio = ( double )width / height;

    emit aspectRatioChanged( aspectRatio );

}

void VideoItem::slotVideoData( uchar *data, unsigned width, unsigned height, int pitch ) {

    if( texture ) {
        texture->deleteLater();
        texture = nullptr;
    }

    QImage::Format frame_format = retroToQImageFormat( pixelFormat );
    QImage image = QImage( data, width, height, pitch, frame_format );

    texture = window()->createTextureFromImage( image, QQuickWindow::TextureOwnsGLTexture );

    texture->moveToThread( window()->openglContext()->thread() );

    // One half of the vsync render loop
    // Invoke a window redraw now that the texture has changed
    update();

}

void VideoItem::emitFrame() {
    emit signalFrame();
}

void VideoItem::handleWindowChanged( QQuickWindow *window ) {

    if( !window ) {
        return;
    }


}

bool VideoItem::limitFrameRate() {
    return false;
}

void VideoItem::setCoreState( Core::State state ) {
    qmlCoreState = state;
    emit coreStateChanged();
}

QSGNode *VideoItem::updatePaintNode( QSGNode *node, UpdatePaintNodeData *paintData ) {
    Q_UNUSED( paintData );

    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>( node );

    if( !textureNode ) {
        textureNode = new QSGSimpleTextureNode;
    }


    if( ( coreState() ) != Core::STATEREADY ) {
        if( coreState() == Core::STATEPAUSED ) {
            textureNode->setTexture( texture );
            textureNode->setRect( boundingRect() );
            textureNode->setFiltering( QSGTexture::Linear );
            textureNode->setTextureCoordinatesTransform( QSGSimpleTextureNode::MirrorVertically |
                    QSGSimpleTextureNode::MirrorHorizontally );
            return textureNode;
        }

        return QQuickItem::updatePaintNode( textureNode, paintData );
    }

    // First frame, no video data yet. Tell core to render a frame
    // then display it next time.
    if( !texture ) {
        //emit signalFrame();
        return QQuickItem::updatePaintNode( textureNode, paintData );
    }

    static qint64 timeStamp = -1;

    if( timeStamp != -1 ) {

        qreal calculatedFrameRate = ( 1 / ( timeStamp / 1000000.0 ) ) * 1000.0;
        int difference = calculatedFrameRate > coreFPS ?
                         calculatedFrameRate - coreFPS :
                         coreFPS - calculatedFrameRate;
        Q_UNUSED( difference );

    }

    timeStamp = frameTimer.nsecsElapsed();
    frameTimer.start();

    textureNode->setTexture( texture );
    textureNode->setRect( boundingRect() );
    textureNode->setFiltering( QSGTexture::Linear );
    textureNode->setTextureCoordinatesTransform( QSGSimpleTextureNode::MirrorVertically |
            QSGSimpleTextureNode::MirrorHorizontally );

    // One half of the vsync loop
    // Now that the texture is sent out to be drawn, tell core to make a new frame
    if( coreState() == Core::STATEREADY ) {
        emit signalFrame();
    }

    return textureNode;

}

QImage::Format VideoItem::retroToQImageFormat( retro_pixel_format fmt ) {

    static QImage::Format format_table[3] = {

        QImage::Format_RGB16,   // RETRO_PIXEL_FORMAT_0RGB1555
        QImage::Format_RGB32,   // RETRO_PIXEL_FORMAT_XRGB8888
        QImage::Format_RGB16    // RETRO_PIXEL_FORMAT_RGB565

    };

    if( fmt >= 0 && fmt < ( sizeof( format_table ) / sizeof( QImage::Format ) ) ) {
        return format_table[fmt];
    }

    return QImage::Format_Invalid;

}
