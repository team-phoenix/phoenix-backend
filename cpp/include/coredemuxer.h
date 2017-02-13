#pragma once

#include "libretro.h"
#include "coresymbols.h"

#include <QObject>
#include <QImage>

class QLibrary;
class QFile;
class QJsonObject;
class QDataStream;
class InputManager;

// The CoreDemuxer's job is to load a core and game, and split up the video and audio streams,
// sending those streams to their destined places.

// Audio data goes straight into the computer's audio output, usually the speaker

// Video data goes into a shared memory region so that other processes can manipulate
// and output this data onto the user's screen.

class CoreDemuxer : public QObject
{
    Q_OBJECT
public:

    enum class State {
        Initial,
        PlayReady,
        Playing,
        Stopping,
        Stopped,
    };

    static CoreDemuxer &instance();
    ~CoreDemuxer();

    void loadLibrary( QString t_lib );
    void loadGame( QString t_game );

public: // Libretro specific variables

    CoreSymbols m_symbols;
    retro_system_av_info m_avInfo;
    QImage::Format m_pixelFormat;

public slots:
    void run();

signals:
    void init();
    void pipeMessage( QByteArray );

private slots:
    void handleInit();
    void handleSocketRead( QJsonObject &t_message );

private: // Functions

    explicit CoreDemuxer( QObject *parent = nullptr );

    void resetSession();
    void shutdownSession();
    bool sessionReady();

    void sendVideoInfo();

    // Send a connected process a message.
    void sendProcessMessage( QString t_section, QString t_value = "" );

    void sendProcessMessage( QJsonObject &&t_message );
    void sendProcessMessage( const QJsonObject &t_message );


private: // Variables

    QByteArray m_partialSocketCmd;
    QLibrary *m_coreLib;
    QFile *m_game;

    QByteArray m_gameData;
    QByteArray m_gameFileName;

    retro_system_info m_systemInfo;

    InputManager *m_inputManager;
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


