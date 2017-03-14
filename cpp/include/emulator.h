#pragma once

#include "libretro.h"
#include "libretrolibrary.h"
#include "messageserver.h"
#include "gamepadmanager.h"

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
public:

    enum class State {
        Uninitialized = 0,
        Initialized,
        Playing,
        Paused,
        Killed,
    };

    static Emulator &instance();
    ~Emulator();

    void setEmuState( State t_state );

public: // Libretro specific variables

    LibretroLibrary m_libretroLibrary;
    retro_system_av_info m_avInfo;
    QImage::Format m_pixelFormat;

    GamepadManager m_gamepadManager;

public slots:
    void run();

    void initializeSession( const QString &t_corePath, const QString &t_gamePath, const QString &hwType );


signals:

    void emulationInitialized();

private slots:
    void shutdownSession();
    void resetSession();
    void killProcess();

private: // Functions

    explicit Emulator( QObject *parent = nullptr );

    void sendVideoInfo();
    void sendState();

    void setCallbacks();


private: // Variables

    State m_emuState;
    QByteArray m_partialSocketCmd;
    QFile m_game;

    QByteArray m_gameData;
    QByteArray m_gameFileName;

    retro_system_info m_systemInfo;

    MessageServer m_messageServer;
    QTimer m_timer;

    bool loadEmulationCore( const QString &t_emuCore );
    bool loadEmulationGame( const QString &t_emuGame );

    QString toString( State t_state );

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


