#pragma once

#include "backendcommon.h"

#include "consumer.h"
#include "controllable.h"
#include "node.h"

#include "audiobuffer.h"

#include "logging.h"

/* The AudioOutput class writes data to the default output device. Its internal buffers must be set by invoking
 * controlIn() with the proper arguments before any data can be passed to it via dataIn(). Set the volume (from 0 to 1
 * inclusive) with slotSetVolume().
 *
 * For clarity, assuming 16-bit stereo audio:
 * 1 frame = 4 bytes (L, L, R, R)
 * 1 sample = 2 bytes (L, L) or (R, R)
 */

class AudioOutput : public Node {
        Q_OBJECT

    public:
        explicit AudioOutput( Node *parent = 0 );
        ~AudioOutput();

    public slots:
        void controlIn( Command command, QVariant data, qint64 timeStamp ) override;
        void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) override;

    private slots:
        void handleStateChanged( QAudio::State state );
        void handleUnderflow();

    private:
        State state{ State::Stopped };
        ProducerFormat format;

        // Respond to the core running or not by keeping audio output active or not
        // AKA we'll pause if core is paused
        void setAudioActive( bool coreIsRunning );

        // Output incoming video frame of audio data to the audio output
        void audioData( int16_t *inputData, int inputBytes );

        // Free memory, clean up
        void shutdown();

        // Completely init/re-init audio output and resampler
        void resetAudio();

        // Allocate memory for conversion
        void allocateMemory();

        // Opaque pointer for libsamplerate
        SRC_STATE *resamplerState;

        // Audio and video timing provided by Core via the controller
        int sampleRate;
        double hostFPS;
        double coreFPS;
        double sampleRateRatio;

        // Internal buffers used for resampling
        short *inputDataShort;
        float *inputDataFloat;
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

        // Max size of the outputBuffer. Equivalent to "audio buffering" setting in other programs
        int outputLengthMs;

        // Ideal amount of data in the output buffer. Make this large enough to ensure no underruns
        int outputTargetMs;

        // Max amount of stretching performed to compensate for output buffer position being off target
        double maxDeviation;

        //
        // ---
        //

};
