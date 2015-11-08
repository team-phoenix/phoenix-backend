#ifndef LIBRETROCORE_H
#define LIBRETROCORE_H

#include "backendcommon.h"

#include "core.h"
#include "libretro.h"
#include "libretrosymbols.h"
#include "libretrovariable.h"
#include "logging.h"

// Since each buffer holds one frame, depending on core, 30 frames = ~500ms
#define POOL_SIZE 30

/*
 * C++ wrapper around a Libretro core. Currently, only one LibretroCore instance may safely exist at any time due to the
 * lack of a context pointer for callbacks to use.
 *
 * The following keys are mandatory for source from setSource():
 * "type": "libretro"
 * "core": Absolute path to the Libretro core
 * "game": Absolute path to a game the Libretro core accepts
 * "systemPath": Absolute path to the system directory (contents of which depend on the core)
 * "savePath": Absolute path to the save directory
 *
 * LibretroCore expects some kind of input producer (such as InputManager) to produce input which LibretroCore will then
 * consume. This input production also drives the production of frames (retro_run()), so time it such that it's as close
 * as possible to the console's native framerate!
 *
 * This also means that you can send updates from the input producer whenever you want, at any stage. retro_run() will
 * not be called unless you're in the PLAYING state.
 */

class LibretroCore : public Core {
        Q_OBJECT

    public:
        explicit LibretroCore( Core *parent = 0 );
        ~LibretroCore();

    signals:
        void libretroCoreNativeFramerate( qreal framerate );
        void libretroCoreNTSC( bool NTSC );

    public slots:
        void consumerFormat( ProducerFormat consumerFmt ) override;
        void consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) override;

        // FIXME: For testing only. Delete once InputManagerProxy is working
        void testDoFrame();

        // State changers
        void load() override;
        void stop() override;

    protected:
        // Only staticly-linked callbacks (and their static helpers) may access this data/call these methods

        // A hack that gives us the implicit C++ 'this' pointer while maintaining a C-style function signature
        // for the callbacks as required by libretro.h. We can only have a single instance of Core running at any time.
        static LibretroCore *core;

        // Struct containing libretro methods
        LibretroSymbols symbols;

        // Used by environment callback. Provides info about the OpenGL context provided by the Phoenix frontend for
        // the core's internal use
        retro_hw_render_callback openGLContext;

        // Used by audio callback
        void emitAudioData( void *data, size_t bytes );

        // Used by video callback
        void emitVideoData( void *data, unsigned width, unsigned height, size_t pitch, size_t bytes );

    private:
        // Files and paths

        QLibrary coreFile;
        QFile gameFile;

        QDir contentPath;
        QDir systemPath;
        QDir savePath;

        // These must be members so their data stays valid throughout the lifetime of LibretroCore
        QFileInfo coreFileInfo;
        QFileInfo gameFileInfo;
        QFileInfo systemPathInfo;
        QFileInfo savePathInfo;
        QByteArray corePathByteArray;
        QByteArray gameFileByteArray;
        QByteArray gamePathByteArray;
        QByteArray systemPathByteArray;
        QByteArray savePathByteArray;
        const char *corePathCString;
        const char *gameFileCString;
        const char *gamePathCString;
        const char *systemPathCString;
        const char *savePathCString;

        // Raw ROM/ISO data, empty if (systemInfo->need_fullpath)
        QByteArray gameData;

        // SRAM

        void *saveDataBuf;
        void loadSaveData();
        void storeSaveData();

        // Core-specific constants

        // Information about the core (we store, Libretro core fills out with symbols.retro_get_system_info())
        retro_system_info *systemInfo;

        // Information about the controllers and buttons used by the core
        // FIXME: Where's the max number of controllers defined?
        // Key format: "port,device,index,id" (all 4 unsigned integers are represented as strings)
        //     ex. "0,0,0,0"
        // Value is a human-readable description
        QMap<QString, QString> inputDescriptors;

        // Producer data

        // Circular buffer pools. Used to avoid having to track when consumers have consumed a buffer
        int16_t *audioBufferPool[ POOL_SIZE ];
        int audioPoolCurrentBuffer;

        // Amount audioBufferPool[ audioBufferPoolIndex ] has been filled
        // Each frame, exactly ( sampleRate * 4 ) bytes should be copied to
        // audioBufferPool[ audioBufferPoolIndex ][ audioBufferCurrentByte ] in total
        // FIXME: In practice, that's not always the case? Some cores only hit that *on average*
        int audioBufferCurrentByte;

        uint8_t *videoBufferPool[ POOL_SIZE ];
        int videoPoolCurrentBuffer;

        // Consumer data

        ProducerFormat consumerFmt;
        int16_t inputStates[ 16 ];

        // Callbacks

        static void audioSampleCallback( int16_t left, int16_t right );
        static size_t audioSampleBatchCallback( const int16_t *data, size_t frames );
        static bool environmentCallback( unsigned cmd, void *data );
        static void inputPollCallback( void );
        static void logCallback( enum retro_log_level level, const char *fmt, ... );
        static int16_t inputStateCallback( unsigned port, unsigned device, unsigned index, unsigned id );
        static void videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch );

        // Misc

        // Core-specific variables
        QMap<std::string, LibretroVariable> variables;

        // Helper that generates key for looking up the inputDescriptors
        QString inputTupleToString( unsigned port, unsigned device, unsigned index, unsigned id );

};

#endif // LIBRETROCORE_H
