#include "audiocontroller.h"

#include "logging.h"

AudioController::AudioController(QObject *parent) : QObject( parent )
{

    m_audioOutput.setBuffer( &m_ringBuffer );
    //m_audioOutput.moveToThread( &m_audioThread );

    connect( this, &AudioController::playEmu, &m_audioOutput, &AudioOutput::emuPlaying );
    connect( this, &AudioController::audioFmtChanged, &m_audioOutput, &AudioOutput::handleAudioFmtChanged );

    //m_audioThread.start( QThread::TimeCriticalPriority );
}

AudioController::~AudioController() {
    //m_audioThread.quit();
    //m_audioThread.wait();
}

void AudioController::write(qint16 t_left, qint16 t_right) {
    qCDebug( phxAudioOutput, "Writing audio sample." );

    uint32_t sample = ( ( uint16_t ) t_left << 16 ) | ( uint16_t ) t_right;

    m_ringBuffer.write( reinterpret_cast<const char *>( &sample ), sizeof( int16_t ) * 2 );
}

void AudioController::write( const qint16 *t_data, size_t t_frameSize) {

    return;
    m_audioOutput.m_device->write( reinterpret_cast<const char *>( t_data ), t_frameSize * sizeof( qint16 ) * 2  );

    //m_ringBuffer.write( reinterpret_cast<const char *>( t_data ), t_frameSize * sizeof( int16_t ) * 2 );
}

