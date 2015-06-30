
#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QAudioOutput>
#include <QDebug>
#include <QBuffer>

#include <memory>
#include <atomic>

#include "audiobuffer.h"
#include "logging.h"

#include "samplerate.h"

/* The AudioOutput class writes data to the default output device. Its internal buffers must be set by calling
 * slotAudioFormat() with the proper arguments before any data can be passed to it with slotAudioData(). In addition,
 * it has the ability to pause and resume whether or not it 'expects' audio with slotSetAudioActive(). Make this match
 * whether or not the core is paused and you'll not have any underruns (hopefully). Set the volume (from 0 to 1
 * inclusive) with slotSetVolume().
 */

class AudioOutput : public QObject {
        Q_OBJECT

    public slots:

        // Tell Audio what sample rate to expect from Core
        void slotAudioFormat( int sampleRate, double coreFPS, double hostFPS );

        // Output incoming video frame of audio data to the audio output
        void slotAudioData( int16_t *inputData, int inputBytes );

        // Respond to the core running or not by keeping audio output active or not
        // AKA Pause if core is paused
        void slotSetAudioActive( bool coreIsRunning );

        // Set volume level [0.0...1.0]
        void slotSetVolume( qreal level );

        void slotShutdown();

    private slots:

        void slotAudioOutputStateChanged( QAudio::State state );

    public:

        AudioOutput();
        ~AudioOutput();

    private:

        // Completely init/re-init audio output
        void resetAudio();

        // Opaque pointer for libsamplerate
        SRC_STATE *resamplerState;

        // Audio and video timing provided by Core via the controller
        int sampleRate;
        double coreFPS;
        double hostFPS;
        double sampleRateRatio;

        // Internal buffers used for resampling
        float *inputDataFloat;
        char *inputDataChar;
        float *outputDataFloat;
        short *outputDataShort;

        // Set to true if the core is currently running
        bool coreIsRunning;

        // Input and output audio formats being used
        QAudioFormat outputAudioFormat;
        QAudioFormat inputAudioFormat;

        // An interface to the output device
        QAudioOutput *outputAudioInterface;

        // Size of outputBuffer's unconsumed data
        int outputCurrentByte;

        // A buffer that removes data from itself once it's read
        AudioBuffer outputBuffer;

        //
        // TODO: Make these configurable
        //

        // Size of the outputBuffer. Currently doesn't really mean much, as outputBuffer can grow
        // to an unlimited size
        int outputLengthMs;

        // Ideal amount of data in the output buffer. Make this large enough to ensure no underruns
        int outputTargetMs;

        // Max amount of stretching performed to compensate for output buffer position being off target
        double maxDeviation;

        //
        // ---
        //


};

#endif
