#pragma once

#include "libretro.h"
#include "libretrolibrary.h"
#include "messageserver.h"
#include "gamepadmanager.h"
#include "sharedmemory.h"
#include "audiocontroller.h"
#include "corevariable.h"
#include "abstractemulator.h"

#include <QImage>
#include <QFile>

#include <QTimer>

class QJsonObject;

// The Emulator's job is to load a core and game, and split up the video and audio streams,
// sending those streams to their destined places.

// Audio data goes straight into the computer's audio output, usually the speaker

// Video data goes into a shared memory region so that other processes can manipulate
// and output this data onto the user's screen.

class Emulator final : public AbstractEmulator {
Q_OBJECT
public:

    explicit Emulator(QObject *parent = nullptr);

    ~Emulator() override;

    void setEmuState(State t_state);

public slots:

    // Runs the Emulator for a single frame.
    // Key calling this function to pump out more frames.
    void run() override;

    // Initializes the Emulator fully.
    // Calling run() directly after this is safe and optimal.
    void init(const QString &t_corePath, const QString &t_gamePath, const QString &hwType) override;

    void shutdown() override;

    void restart() override;

    // Kills this Emulator server process.
    // So long fellow!
    void kill() override;

private: // Functions

    virtual void routeAudioBatch( const qint16* data, size_t frames ) override {
        m_audioController.write(data, frames);
    }

    virtual void routeAudioSample( qint16 left, qint16 right ) override {
        m_audioController.write(left, right);
    }

    void routeVideoFrame( const char *data, quint32 width, quint32 height, size_t pitch ) override {
        m_sharedMemory.writeVideoFrame( width, height, pitch, data );
    }

    void sendVariables();

    // Sends a JSON data package to any listening sockets.
    void sendVideoInfo();

    // Sends the Emulator's current state to and listening sockets.
    void sendState();

    void setCallbacks();


private: // Variables

    State      m_emuState;
    QFile      m_game;

    QByteArray m_gameData;
    QByteArray m_gameFileName;

    retro_system_info m_systemInfo;

    MessageServer m_messageServer;
    QTimer        m_timer;

    QHash<QByteArray, QByteArray> m_coreVars;


    bool loadEmulationCore(const QString &t_emuCore);

    bool loadEmulationGame(const QString &t_emuGame);

    QString toString(State t_state);

private slots:

    void handleVariableUpdate(const QByteArray &t_key, const QByteArray &t_value);

};