#pragma once

#include "libretro.h"
#include "libretrolibrary.h"
#include "messageserver.h"
#include "gamepadmanager.h"
#include "sharedmemory.h"
#include "audiocontroller.h"
#include "corevariable.h"

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
    Q_ENUM( State )

    explicit Emulator( QObject *parent = nullptr );

    static Emulator* instance();
     ~Emulator();

     void setEmuState( State t_state );

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
     void runEmu();

    // Initializes the Emulator fully.
    // Calling runEmu() directly after this is safe and optimal.
     void initEmu( const QString &t_corePath, const QString &t_gamePath, const QString &hwType );

     void shutdownEmu();

     void restartEmu();

    // Kills this Emulator server process.
    // So long fellow!
     void killEmu();

private: // Functions


     void sendVariables();

    // Sends a JSON data package to any listening sockets.
     void sendVideoInfo();

    // Sends the Emulator's current state to and listening sockets.
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

    QHash<QByteArray, QByteArray> m_coreVars;


     bool loadEmulationCore( const QString &t_emuCore );
     bool loadEmulationGame( const QString &t_emuGame );

     QString toString( State t_state );

private slots:

     void handleVariableUpdate( const QByteArray &t_key, const QByteArray &t_value );

};