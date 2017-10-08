#pragma once

#include "libretro.h"
#include "libretrolibrary.h"
#include "messageserver.h"
#include "gamepadmanager.h"
#include "sharedmemory.h"
#include "audiocontroller.h"
#include "corevariable.h"
#include "testing.h"

#include <QObject>
#include <QImage>
#include <QFile>

#include <QTimer>

class QJsonObject;

// The Emulator's job is to load a core and game, and split up the video and audio streams,
// sending those streams to their destined places.

// Audio data goes straight into the computer's audio output, usually the speaker

// Video data goes into a shared memory region so that other processes can manipulate
// and output this data onto the user's screen.

class Emulator : public QObject
{
    Q_OBJECT
    friend class Test_Emulator;

public:

    enum class State {
        Uninitialized = 0,
        Initialized,
        Playing,
        Paused,
        Killed,
    };
    Q_ENUM( State )

    explicit Emulator( QObject *parent = nullptr );

    static Emulator* instance();
    MOCKABLE ~Emulator();

    MOCKABLE void setEmuState( State t_state );

    // This member variables need to be public so they can be accessed
    // by the static callbacks.
public: // Libretro specific variables

    LibretroLibrary m_libretroLibrary;
    retro_system_av_info m_avInfo;
    QImage::Format m_pixelFormat;

    VariableModel m_variableModel;

    GET_SET( Emulator, bool, coreVarsUpdated )

public:

    GamepadManager m_gamepadManager;
    SharedMemory m_sharedMemory;
    AudioController m_audioController;

public slots:

    // Runs the Emulator for a single frame.
    // Key calling this function to pump out more frames.
    MOCKABLE void runEmu();

    // Initializes the Emulator fully.
    // Calling runEmu() directly after this is safe and optimal.
    MOCKABLE void initEmu( const QString &t_corePath, const QString &t_gamePath, const QString &hwType );

    MOCKABLE void shutdownEmu();

    MOCKABLE void restartEmu();

    // Kills this Emulator server process.
    // So long fellow!
    MOCKABLE void killEmu();

private: // Functions


    MOCKABLE void sendVariables();

    // Sends a JSON data package to any listening sockets.
    MOCKABLE void sendVideoInfo();

    // Sends the Emulator's current state to and listening sockets.
    MOCKABLE void sendState();

    MOCKABLE void setCallbacks();


private: // Variables

    State m_emuState;
    QByteArray m_partialSocketCmd;
    QFile m_game;

    QByteArray m_gameData;
    QByteArray m_gameFileName;

    retro_system_info m_systemInfo;

    MessageServer m_messageServer;
    QTimer m_timer;

    QHash<QByteArray, QByteArray> m_coreVars;


    MOCKABLE bool loadEmulationCore( const QString &t_emuCore );
    MOCKABLE bool loadEmulationGame( const QString &t_emuGame );

    MOCKABLE QString toString( State t_state );

private slots:

    MOCKABLE void handleVariableUpdate( const QByteArray &t_key, const QByteArray &t_value );

};

// Callbacks
static void audioSampleCallback( int16_t left, int16_t right );

static size_t audioSampleBatchCallback( const int16_t *data, size_t frames );

static bool environmentCallback( unsigned cmd, void *data );

static void inputPollCallback( void );

static void logCallback( enum retro_log_level level, const char *fmt, ... );

static int16_t inputStateCallback( unsigned port, unsigned device, unsigned index, unsigned id );

static void videoRefreshCallback( const void *data, unsigned width, unsigned height, size_t pitch );

// Extra callbacks
static uintptr_t getFramebufferCallback( void );

static retro_proc_address_t openGLProcAddressCallback( const char *sym );

static bool rumbleCallback( unsigned port, enum retro_rumble_effect effect, uint16_t strength );


