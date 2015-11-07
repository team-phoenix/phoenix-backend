#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include "backendcommon.h"

#include "consumer.h"
#include "controllable.h"

#include "audiobuffer.h"

#include "logging.h"

/* The AudioOutput class writes data to the default output device. Its internal buffers must be set by calling
 * slotAudioFormat() with the proper arguments before any data can be passed to it with slotAudioData(). In addition,
 * it has the ability to pause and resume whether or not it 'expects' audio with slotSetAudioActive(). Make this match
 * whether or not the core is paused and you'll not have any underruns (hopefully). Set the volume (from 0 to 1
 * inclusive) with slotSetVolume().
 */

class AudioOutput : public QObject, public Consumer, public Controllable {
        Q_OBJECT

    public:
        explicit AudioOutput( QObject *parent = 0 );
        ~AudioOutput();

    public slots:
        void consumerFormat( ProducerFormat consumerFmt ) override;
        void consumerData( QString type, QMutex *mutex, void *data, size_t bytes , qint64 timestamp ) override;
        void setState( Control::State currentState ) override;

        // Tell AudioOutput what the framerate driving the frame production is as it might differ from the core's
        // native framerate. This will automatically compensate if it does
        void libretroSetFramerate( qreal hostFPS );

    private slots:
        void slotAudioOutputStateChanged( QAudio::State currentState );

    private:
        // Respond to the core running or not by keeping audio output active or not
        // AKA Pause if core is paused
        void setAudioActive( bool coreIsRunning );

        // Output incoming video frame of audio data to the audio output
        void audioData( int16_t *inputData, int inputBytes );

        // Free memory, clean up
        void shutdown();

        // Completely init/re-init audio output
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
