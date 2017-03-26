#pragma once

#include <QIODevice>
#include <QThread>

#include <QAudioFormat>

#include "audiooutput.h"
#include "ringbuffer.h"

class AudioController : public QObject
{
    Q_OBJECT
public:
    explicit AudioController( QObject *parent = nullptr );
    ~AudioController();

    void write(qint16 t_left, qint16 t_right);
    void write( const qint16 *t_data, size_t t_frameSize );

signals:
    void playEmu();
    void audioFmtChanged( double t_fps, double t_sampleRate );

private slots:
    void setIsDisabled( bool t_disabled ) {
        m_isDisabled = t_disabled;
    }

private:

    AudioOutput m_audioOutput;
    QThread m_audioThread;

    RingBuffer m_ringBuffer;

    bool m_isDisabled;

};
