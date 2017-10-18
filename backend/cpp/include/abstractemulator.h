#pragma once

#include <QObject>
#include <QtGui/QImage>

#include "macros.h"
#include "audiocontroller.h"
#include "gamepadmanager.h"

#include "libretrolibrary.h"
#include "libretro.h"
#include "corevariable.h"
#include "sharedmemory.h"

class AbstractEmulator : public QObject {
    Q_OBJECT
public:

    enum class State {
        Uninitialized = 0,
        Initialized,
        Playing,
        Paused,
        Killed,
    };

    Q_ENUM(State)

    explicit AbstractEmulator( QObject *parent = nullptr ) : QObject( parent ) {}

    virtual ~AbstractEmulator() {}

    virtual void run() = 0;
    virtual void init(const QString &t_corePath, const QString &t_gamePath, const QString &hwType) = 0;
    virtual void kill() = 0;
    virtual void restart() = 0;
    virtual void shutdown() = 0;

    virtual void routeAudioBatch( const qint16* data, size_t frames ) = 0;
    virtual void routeAudioSample( qint16 left, qint16 right ) = 0;

    virtual void routeVideoFrame( const char *data, quint32 width, quint32 height, size_t pitch ) = 0;


    // This member variables need to be public so they can be accessed
    // by the static callbacks.
public: // Libretro specific variables

    LibretroLibrary      m_libretroLibrary;
    retro_system_av_info m_avInfo;
    QImage::Format       m_pixelFormat{ QImage::Format_Invalid };

    VariableModel m_variableModel;

    GET_SET(AbstractEmulator, bool, coreVarsUpdated)

public:
    GamepadManager  m_gamepadManager;
    SharedMemory    m_sharedMemory;
    AudioController m_audioController;

};
