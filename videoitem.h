#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include "backendcommon.h"

#include "libretro.h"
#include "libretrocore.h"
#include "audiooutput.h"
#include "logging.h"

#include "input/inputmanager.h"

class VideoItem : public QQuickItem {
        Q_OBJECT
        Q_PROPERTY( QString libretroCore MEMBER corePath WRITE setCore )
        Q_PROPERTY( QString game MEMBER gamePath WRITE setGame )
        Q_PROPERTY( InputManager *inputManager READ inputManager WRITE setInputManager NOTIFY inputManagerChanged )
        Q_PROPERTY( double aspectRatio MEMBER aspectRatio NOTIFY aspectRatioChanged )
        Q_PROPERTY( bool running READ running NOTIFY signalRunChanged )
        Q_PROPERTY( LibretroCore::State coreState READ coreState NOTIFY coreStateChanged )

    public:

        VideoItem( QQuickItem *parent = 0 );
        ~VideoItem();

        InputManager *inputManager() const;

        void setInputManager( InputManager *manager );

        static void registerTypes();

        // QML Property Getters
        bool running() const;

        LibretroCore::State coreState() const;

    signals:

        // Controller
        void signalLoadCore( QString path );
        void signalLoadGame( QString path );
        void signalAudioFormat( int sampleRate, double coreFPS, double hostFPS );
        void signalVideoFormat( retro_pixel_format pixelFormat, int width, int height, int pitch,
                                double coreFPS, double hostFPS );
        void signalFrame();
        void signalShutdown();
        void signalRunChanged( bool run );
        void signalChangeCoreThread( QThread *thread );

        void signalDevice( InputDevice *device );
        void inputManagerChanged();

        void coreStateChanged();

        void signalSetVolume( qreal level );

        // Consumer
        void aspectRatioChanged( double aspectRatio );

    private slots:
        void setRunning( const bool running );

    public slots:

        // Controller
        void slotCoreStateChanged( LibretroCore::State newState, LibretroCore::Error error );
        void slotCoreAVFormat( retro_system_av_info avInfo, retro_pixel_format pixelFormat );
        void slotPause();
        void slotResume();
        void slotStop();

        // Consumer
        void slotVideoFormat( retro_pixel_format pixelFormat, int width, int height, int pitch,
                              double coreFPS, double hostFPS );
        void slotVideoData( uchar *data, unsigned width, unsigned height, int pitch );
        void emitFrame();

    private slots:

        // For future consumer use?
        void handleWindowChanged( QQuickWindow *window );

    private:

        // QML Varibles
        bool qmlRunning;

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
        LibretroCore *core;

        // The timer that makes the core produce frames at regular intervals
        // QTimer *coreTimer;

        // Thread that keeps the emulation from blocking this UI thread
        QThread *coreThread;
        QTimer coreTimer;

        // Core's 'current' state (since core lives on another thread, it could be in a different state)
        LibretroCore::State qmlCoreState;
        void setCoreState( LibretroCore::State state );

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
        double aspectRatio;

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
Q_DECLARE_METATYPE( LibretroCore::State )
Q_DECLARE_METATYPE( LibretroCore::Error )

#endif // VIDEOITEM_H
