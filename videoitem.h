#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QQuickItem>
#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QImage>
#include <QQueue>
#include <QElapsedTimer>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>
#include <QSGSimpleTextureNode>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

#include <memory>

#include "libretro.h"
#include "core.h"
#include "audiooutput.h"
#include "logging.h"

#include "input/inputmanager.h"

class VideoItem : public QQuickItem {
        Q_OBJECT
        Q_PROPERTY( QString libretroCore MEMBER corePath WRITE setCore )
        Q_PROPERTY( QString game MEMBER gamePath WRITE setGame )
        Q_PROPERTY( InputManager *inputManager READ inputManager WRITE setInputManager NOTIFY inputManagerChanged )

    public:

        VideoItem( QQuickItem *parent = 0 );
        ~VideoItem();

        InputManager *inputManager() const;

        void setInputManager( InputManager *manager );

        static void registerTypes();

    signals:
        void inputManagerChanged();
        void signalDevice( InputDevice *device );
        // Controller
        void signalLoadCore( QString path );
        void signalLoadGame( QString path );
        void signalAudioFormat( int sampleRate, double coreFPS, double hostFPS );
        void signalVideoFormat( retro_pixel_format pixelFormat, int width, int height, int pitch,
                                double coreFPS, double hostFPS );
        void signalFrame();
        void signalShutdown();
        void signalRunChanged( bool run );

    public slots:

        // Controller
        void slotCoreStateChanged( Core::State newState, Core::Error error );
        void slotCoreAVFormat( retro_system_av_info avInfo, retro_pixel_format pixelFormat );

        // Consumer
        void slotVideoFormat( retro_pixel_format pixelFormat, int width, int height, int pitch,
                              double coreFPS, double hostFPS );
        void slotVideoData( uchar *data, unsigned width, unsigned height, int pitch );

        void emitFrame()
        {
            emit signalFrame();
        }

    private slots:

        // For future consumer use?
        void handleWindowChanged( QQuickWindow *window );

    private:

        InputManager *qmlInputManager;

        //
        // Controller
        //

        bool limitFrameRate();

        // NOTE: All consumers must be declared before Core

        // Audio output on the system's default audio output device
        AudioOutput *audioOutput;

        // Resampling is an expensive operation, keep it on a seperate thread, too
        QThread *audioOutputThread;

        // The emulator itself, a libretro core
        Core *core;

        // The timer that makes the core produce frames at regular intervals
        // QTimer *coreTimer;

        // Thread that keeps the emulation from blocking this UI thread
        QThread *coreThread;
        QTimer coreTimer;

        // Core's 'current' state (since core lives on another thread, it could be in a different state)
        Core::State coreState;

        // Timing and format information provided by core once the core/game is loaded
        // Needs to be passed down to all consumers via signals
        // Don't use these in consumer methods, keep the separation alive...
        retro_system_av_info avInfo;
        retro_pixel_format pixelFormat;

        // Paths to the core and game provided by the QML side of things
        // The setters invoke the core directly, both must be called to start emulation
        QString corePath;
        QString gamePath;
        void setGame( QString game );
        void setCore( QString libretroCore );

        //
        // Consumer
        //

        // Video format info
        int width, height, pitch;
        double coreFPS, hostFPS;

        // The texture that will hold video frames from core
        QSGTexture *texture;

        // Applies texture to the QML item. Since this is called during vsync, we can rely on this being called at
        // approximately the refresh rate of the monitor. Thus, we'll emit the render signal to core here
        QSGNode *updatePaintNode( QSGNode *node, UpdatePaintNodeData *paintData );

        // Timer used to measure FPS of core
        QElapsedTimer frameTimer;

        //
        // Consumer helpers
        //

        // Small helper method to convert Libretro image format types to their Qt equivalent
        inline QImage::Format retroToQImageFormat( enum retro_pixel_format fmt );

};

Q_DECLARE_METATYPE( retro_system_av_info )
Q_DECLARE_METATYPE( retro_pixel_format )
Q_DECLARE_METATYPE( Core::State )
Q_DECLARE_METATYPE( Core::Error )

#endif // VIDEOITEM_H
