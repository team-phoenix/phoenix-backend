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
 * C++ wrapper around a Libretro core. Currently, only one LibretroCore instance may exist at any time due to the
 * lack of a context pointer for callbacks to use.
 *
 * The following keys are mandatory for source from setSource():
 * "type": "libretro"
 * "core": Absolute path to the Libretro core
 * "game": Absolute path to a game the Libretro core accepts
 * "systemPath": Absolute path to the system directory (contents of which depend on the core)
 * "savePath": Absolute path to the save directory
 */

class LibretroCore : public Core {
    public:
        explicit LibretroCore( Core *parent = 0 );

    signals:

    public slots:
        void setSource( QMap<QString, QString> source );

        // State changers
        void load();
        void play();
        void pause();
        void stop();

    protected:
        // Only staticly-linked callbacks may access this data/call these methods

        // A hack that gives us the implicit C++ 'this' pointer while maintaining a C-style function signature
        // for the callbacks as required by libretro.h. Thanks to this, at this time we can only
        // have a single instance of Core running at any time.
        static LibretroCore *core;

        // Struct containing libretro methods
        LibretroSymbols symbols;

        // Used by environment callback. Provides info about the OpenGL context provided by the Phoenix frontend for
        // the core's internal use
        retro_hw_render_callback openGLContext;

        // Used by audio callback
        void emitAudioData( void *data, int bytes );

        // Used by video callback
        void emitVideoData( void *data, int bytes );

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
        const char *corePathCString;
        const char *gameFileCString;
        const char *gamePathCString;
        const char *systemPathCString;
        const char *savePathCString;

        // Raw ROM/ISO data, empty if (fullPathNeeded)
        QByteArray gameData;

        // SRAM

        void *saveDataBuf;
        void loadSaveData();
        void storeSaveData();

        // Core-specific constants

        // Audio and video rates/dimensions/types
        retro_system_av_info *avInfo;
        retro_pixel_format pixelFormat;

        // Information about the core
        retro_system_info *systemInfo;

        // Information about the controllers and buttons used by the core
        // FIXME: Where's the max number of controllers defined?
        QMap< QString, QString > inputDescriptors;

        // Buffer pools (producer)

        int16_t *audioBufferPool[POOL_SIZE];
        int audioPoolCurrentBuffer;
        uchar *videoBufferPool[POOL_SIZE];
        int videoPoolCurrentBuffer;

        // Amount audioBufferPool[ audioBufferPoolIndex ] has been filled
        // Each frame, exactly ( sampleRate * 4 ) bytes should be copied to
        // audioBufferPool[ audioBufferPoolIndex ][ audioBufferCurrentByte ] in total
        int audioBufferCurrentByte;

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
        QMap< std::string, LibretroVariable > variables;

        // Helper that generates key for looking up the inputDescriptors
        QString inputDescriptorKey( unsigned port, unsigned device, unsigned index, unsigned id );

};

#endif // LIBRETROCORE_H
