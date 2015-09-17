#include "videoitem.h"

VideoItem::VideoItem( QQuickItem *parent ) :
    QQuickItem( parent ),
    qmlInputManager( nullptr ),
    audioOutput( new AudioOutput() ), audioOutputThread( new QThread( this ) ),
    core( new Core() ), // coreTimer( new QTimer() ),
    coreThread( nullptr ), coreState( Core::STATEUNINITIALIZED ),
    avInfo(), pixelFormat(),
    corePath( "" ), gamePath( "" ),
    width( 0 ), height( 0 ), pitch( 0 ), coreFPS( 0.0 ), hostFPS( 0.0 ), aspectRatio( 1.0 ),
    texture( nullptr ),
    frameTimer() {

    setFlag( QQuickItem::ItemHasContents, true );

    // Place the objects under VideoItem's control into their own threads
    audioOutput->moveToThread( audioOutputThread );

    // Ensure the objects are cleaned up when it's time to quit and destroyed once their thread is done
    connect( this, &VideoItem::signalShutdown, audioOutput, &AudioOutput::slotShutdown );
    connect( this, &VideoItem::signalShutdown, core, &Core::slotShutdown );
    connect( audioOutputThread, &QThread::finished, audioOutput, &AudioOutput::deleteLater );

    // Catch the user exit signal and clean up
    connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [ = ]() {

        qCDebug( phxController ) << "===========QCoreApplication::aboutToQuit()===========";

        // Shut down Core and the consumers
        emit signalShutdown();

        // Stop processing events in the other threads, then block the main thread until they're finished

        // Stop consumer threads
        audioOutputThread->exit();
        audioOutputThread->wait();
        audioOutputThread->deleteLater();

    } );

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

    // Don't let the Qt police find out we're declaring these structs as metatypes
    // without proper constructors/destructors declared/written
    qRegisterMetaType<retro_system_av_info>();
    qRegisterMetaType<retro_pixel_format>();
    qRegisterMetaType<Core::State>();
    qRegisterMetaType<Core::Error>();
}

void VideoItem::slotCoreStateChanged( Core::State newState, Core::Error error ) {

    qCDebug( phxController ) << "slotStateChanged(" << Core::stateToText( newState ) << "," << error << ")";

    coreState = newState;

    switch( newState ) {

        case Core::STATEUNINITIALIZED:
            break;

        // Time to run the game
        case Core::STATEREADY:

            // This is mixing control (coreThread) and consumer (render thread) members...
            coreThread = window()->openglContext()->thread();


            // Run a timer to make core produce a frame at regular intervals
            // Disabled at the moment due to the granulatiry being 1ms (not good enough)

            //            // Set up and start the frame timer
            //            qCDebug( phxController ) << "coreTimer.start("
            //                                     << ( double )1 / ( avInfo.timing.fps / 1000 )
            //                                     << "ms (core) =" << ( int )( 1 / ( avInfo.timing.fps / 1000 ) )
            //                                     << "ms (actual) )";

            //            // Stop when the program stops
            //            connect( this, &VideoItem::signalShutdown, coreTimer, &QTimer::stop );

            //            // Millisecond accuracy on Unix (OS X/Linux)
            //            // Multimedia timer accuracy on Windows (better?)
            //            coreTimer->setTimerType( Qt::PreciseTimer );

            //            // Granulatiry is in the integer range :(
            //            coreTimer->start( ( int )( 1 / ( avInfo.timing.fps / 1000 ) ) );

            //            // Have the timer run in the same thread as Core
            //            // This will mean timeouts are blocking, preventing them from piling up if Core runs too slow
            //            coreTimer->moveToThread( coreThread );
            //            connect( coreThread, &QThread::finished, coreTimer, &QTimer::deleteLater );

            // Place Core into the render thread
            // Mandatory for OpenGL cores
            // Also prevents massive overhead/performance loss caused by QML effects (like FastBlur)

            core->moveToThread( coreThread );
            connect( coreThread, &QThread::finished, core, &Core::deleteLater );

            qCDebug( phxController ) << "Begin emulation.";

            // Let all the consumers know emulation began
            emit signalRunChanged( true );

            // Get core to immediately (sorta) produce the first frame
            emit signalFrame();

            // Force an update to keep the render thread from pausing
            update();

            break;

        case Core::STATEFINISHED:
            break;

        case Core::STATEERROR:
            switch( error ) {
                default:
                    break;
            }

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

void VideoItem::setCore( QString libretroCore ) {

    corePath = libretroCore;//QUrl( libretroCore ).toLocalFile();
    qDebug() << corePath << libretroCore;

    // qCDebug( phxController ) << "emit signalLoadCore(" << corePath << ")";
    emit signalLoadCore( corePath );

}

void VideoItem::setGame( QString game ) {

    gamePath = game;// QUrl( game ).toLocalFile();

    // qCDebug( phxController ) << "emit signalLoadGame(" << gamePath << ")";
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
    this->aspectRatio = (double)width / height;

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

void VideoItem::handleWindowChanged( QQuickWindow *window ) {

    if( !window ) {
        return;
    }


}

bool VideoItem::limitFrameRate() {
    return false;
}

QSGNode *VideoItem::updatePaintNode( QSGNode *node, UpdatePaintNodeData *paintData ) {
    Q_UNUSED( paintData );

    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>( node );

    if( !textureNode ) {
        textureNode = new QSGSimpleTextureNode;
    }


    // It's not time yet. Show a black rectangle.
    if( coreState != Core::STATEREADY ) {
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
    if( coreState == Core::STATEREADY ) {
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
