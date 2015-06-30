
#include "audiooutput.h"

const auto samplesPerFrame = 2;
// const auto framesPerSample = 0.5;

AudioOutput::AudioOutput()
    : resamplerState( nullptr ),
      inputDataFloat( nullptr ), inputDataChar( nullptr ),
      outputDataFloat( nullptr ), outputDataShort( nullptr ),
      coreIsRunning( false ),
      outputAudioInterface( nullptr ),
      outputCurrentByte( 0 ),
      outputBuffer( this ),
      outputLengthMs( 64 ), outputTargetMs( 32 ), maxDeviation( 0.005 ) {

    outputBuffer.start();

}

AudioOutput::~AudioOutput() {

}

//
// Public slots
//

void AudioOutput::slotAudioFormat( int sampleRate, double coreFPS, double hostFPS ) {

    qCDebug( phxAudioOutput, "slotAudioFormat( %i Hz, %f fps (core), %f fps (host) )", sampleRate, coreFPS, hostFPS );

    this->sampleRate = sampleRate;
    this->coreFPS = coreFPS;
    this->hostFPS = hostFPS;

    inputAudioFormat.setSampleSize( 16 );
    inputAudioFormat.setSampleRate( sampleRate );
    inputAudioFormat.setChannelCount( 2 );
    inputAudioFormat.setSampleType( QAudioFormat::SignedInt );
    inputAudioFormat.setByteOrder( QAudioFormat::LittleEndian );
    inputAudioFormat.setCodec( "audio/pcm" );

    // Try using the nearest supported format
    QAudioDeviceInfo info( QAudioDeviceInfo::defaultOutputDevice() );
    outputAudioFormat = info.nearestFormat( inputAudioFormat );

    // If that got us a format with a worse sample rate, use preferred format
    if( outputAudioFormat.sampleRate() <= inputAudioFormat.sampleRate() ) {
        outputAudioFormat = info.preferredFormat();
    }

    // Force 16-bit audio (for now)
    outputAudioFormat.setSampleSize( 16 );

    sampleRateRatio = ( qreal )outputAudioFormat.sampleRate()  / inputAudioFormat.sampleRate();

    qCDebug( phxAudioOutput ) << "audioFormatIn" << inputAudioFormat;
    qCDebug( phxAudioOutput ) << "audioFormatOut" << outputAudioFormat;
    qCDebug( phxAudioOutput ) << "sampleRateRatio" << sampleRateRatio;
    qCDebug( phxAudioOutput, "Using nearest format supported by sound card: %iHz %ibits",
             outputAudioFormat.sampleRate(), outputAudioFormat.sampleSize() );

    resetAudio();

}

void AudioOutput::slotAudioData( int16_t *inputDataShort, int inputBytes ) {

    // Handle the situation where there is an error opening the audio device
    if( outputAudioInterface->error() == QAudio::OpenError ) {
        qWarning( phxAudioOutput ) << "QAudio::OpenError, attempting reset...";
        resetAudio();
    }

    int inputFrames = inputAudioFormat.framesForBytes( inputBytes );
    int inputSamples = inputFrames * samplesPerFrame;

    // What do we have to work with?
    int outputTotalBytes = outputAudioFormat.bytesForDuration( outputLengthMs * 1000 );
    outputCurrentByte = outputBuffer.bytesAvailable();
    int outputFreeBytes = outputTotalBytes - outputCurrentByte;
    int outputFreeFrames = outputAudioFormat.framesForBytes( outputFreeBytes );
    int outputFreeSamples = outputFreeFrames * samplesPerFrame;
    Q_UNUSED( outputTotalBytes );
    Q_UNUSED( outputFreeSamples );

    // Calculate how much the read data should be scaled (shrunk or stretched) to keep the buffer on target
    int outputTargetByte = outputAudioFormat.bytesForDuration( outputTargetMs * 1000 );
    int outputVectorTargetToCurrent = outputTargetByte - outputCurrentByte;
    double bufferTargetCorrectionRatio = ( double )outputVectorTargetToCurrent / outputTargetByte;

    // Calculate the final DRC ratio
    double DRCRatio = 1.0 + maxDeviation * bufferTargetCorrectionRatio;
    double adjustedSampleRateRatio = sampleRateRatio * DRCRatio * ( hostFPS / coreFPS );

    // libsamplerate works in floats, must convert to floats for processing
    src_short_to_float_array( ( short * )inputDataShort, inputDataFloat, inputSamples );

    // Set up a struct containing parameters for the resampler
    SRC_DATA srcData;
    srcData.data_in = inputDataFloat;
    srcData.data_out = outputDataFloat;
    srcData.end_of_input = 0;
    srcData.input_frames = inputFrames;
    srcData.output_frames = outputFreeFrames; // Max size
    srcData.src_ratio = adjustedSampleRateRatio;

    // Perform resample
    src_set_ratio( resamplerState, adjustedSampleRateRatio );
    auto errorCode = src_process( resamplerState, &srcData );

    if( errorCode ) {
        qCWarning( phxAudioOutput ) << "libresample error: " << src_strerror( errorCode ) ;
    }

    auto outputFramesConverted = srcData.output_frames_gen;
    auto outputBytesConverted = outputAudioFormat.bytesForFrames( outputFramesConverted );
    auto outputSamplesConverted = outputFramesConverted * samplesPerFrame;

    // Convert float data back to shorts
    src_float_to_short_array( outputDataFloat, outputDataShort, outputSamplesConverted );

    // Send the converted data out
    int outputBytesWritten = outputBuffer.write( ( char * ) outputDataShort, outputBytesConverted );
    Q_UNUSED( outputBytesWritten );
    outputCurrentByte += outputBytesWritten;

    //    qCDebug( phxAudioOutput ) << "Output is" << ( ( ( double )( ( outputTotalBytes - outputFreeBytes ) ) /
    //                              outputTotalBytes ) * 100 )
    //                              << "% full" << "(target:" << ( ( double )outputTargetMs / outputLengthMs ) * 100 << "%)";
    //    qCDebug( phxAudioOutput ) << "DRCRatio =" << DRCRatio
    //                              << "outputAudioInterface->bufferSize()" << outputAudioInterface->bufferSize();
    //    qCDebug( phxAudioOutput ) << "\toutputTotalBytes =" << outputTotalBytes
    //                              << "outputCurrentByte =" << outputCurrentByte
    //                              << " outputFreeBytes =" << outputFreeBytes
    //                              << "inputBytes =" << inputBytes;
    //    qCDebug( phxAudioOutput ) << "\tOutput buffer is" << outputAudioFormat.durationForBytes( outputTotalBytes ) / 1000
    //                              << "ms, target =" << outputTargetMs
    //                              << "ms (" << ( double )100.0 * ( ( double )outputTargetMs / (
    //                                          ( double )( outputAudioFormat.durationForBytes(
    //                                                  outputTotalBytes ) ) / 1000.0 ) )
    //                              << "%)";
    //    qCDebug( phxAudioOutput ) << "\tOutput: needed" << outputVectorTargetToCurrent << "bytes, wrote"
    //                              << outputBytesWritten << "bytes";
    //    qCDebug( phxAudioOutput ) << "\toutputTargetByte =" << outputTargetByte
    //                              << "outputVectorTargetToCurrent =" << outputVectorTargetToCurrent;
    //    qCDebug( phxAudioOutput ) << "\toutputAudioInterface->bufferSize() =" << outputAudioInterface->bufferSize()
    //                              << "outputAudioInterface->bytesFree() =" << outputAudioInterface->bytesFree()
    //                              << "outputBuffer.bytesToWrite() =" << outputBuffer.bytesToWrite();
    //    qCDebug( phxAudioOutput ) << "\toutputBuffer.bytesAvailable() =" << outputBuffer.bytesAvailable()
    //                              << "outputBuffer.size() =" << outputBuffer.size()
    //                              << "outputBuffer.pos() =" << outputBuffer.pos();
    //    qCDebug( phxAudioOutput ) << "\tState:" << outputAudioInterface->state()
    //                              << " error: " << outputAudioInterface->error();

}

void AudioOutput::slotSetAudioActive( bool coreIsRunning ) {

    this->coreIsRunning = coreIsRunning;

    if( !outputAudioInterface ) {
        return;
    }

    if( !coreIsRunning ) {
        if( outputAudioInterface->state() != QAudio::SuspendedState ) {
            qCDebug( phxAudioOutput ) << "Paused";
            outputAudioInterface->suspend();
        }
    } else {
        if( outputAudioInterface->state() != QAudio::ActiveState ) {
            qCDebug( phxAudioOutput ) << "Started";
            outputAudioInterface->resume();
        }
    }

}

void AudioOutput::slotSetVolume( qreal level ) {

    if( outputAudioInterface ) {
        outputAudioInterface->setVolume( level );
    }

}

void AudioOutput::slotShutdown() {

    qCDebug( phxAudioOutput ) << "slotShutdown() start";

    if( outputAudioInterface ) {
        outputAudioInterface->stop();
        delete outputAudioInterface;
        outputAudioInterface = nullptr;
    }

    if( outputDataFloat ) {
        delete outputDataFloat;
        outputDataFloat = nullptr;
    }

    if( outputDataShort ) {
        delete outputDataShort;
        outputDataShort = nullptr;
    }

    qCDebug( phxAudioOutput ) << "slotShutdown() end";

}

//
// Private slots
//

void AudioOutput::slotAudioOutputStateChanged( QAudio::State s ) {

    if( s == QAudio::IdleState && outputAudioInterface->error() == QAudio::UnderrunError ) {
        qWarning( phxAudioOutput ) << "audioOut underrun";

        if( outputAudioInterface ) {
            outputAudioInterface->start( &outputBuffer );
        } else {
            resetAudio();
        }
    }

    if( s != QAudio::IdleState && s != QAudio::ActiveState ) {
        qCDebug( phxAudioOutput ) << "State changed:" << s;
    }

}

//
// Private
//

void AudioOutput::resetAudio() {

    // Reset the resampler

    if( resamplerState ) {
        src_delete( resamplerState );
        resamplerState = nullptr;
    }

    int errorCode;
    resamplerState = src_new( SRC_SINC_BEST_QUALITY, 2, &errorCode );

    if( !resamplerState ) {
        qCWarning( phxAudioOutput ) << "libresample could not init: " << src_strerror( errorCode ) ;
    }

    // Reset the output interface object

    if( outputAudioInterface ) {
        outputAudioInterface->stop();
        delete outputAudioInterface;
        outputAudioInterface = nullptr;
    }

    outputAudioInterface = new QAudioOutput( outputAudioFormat, this );
    Q_CHECK_PTR( outputAudioInterface );

    connect( outputAudioInterface, &QAudioOutput::stateChanged, this, &AudioOutput::slotAudioOutputStateChanged );
    outputAudioInterface->start( &outputBuffer );
    QAudio::State state = outputAudioInterface->state();
    qCDebug( phxAudioOutput ) << state;

    if( !coreIsRunning ) {
        outputAudioInterface->suspend();
    }

    // Reallocate space for scratch space conversion buffers

    auto outputBufferSizeSamples = outputAudioFormat.bytesForDuration( outputLengthMs * 1000 );
    qCDebug( phxAudioOutput ) << "Allocated" << outputBufferSizeSamples * 2 * 2 / 1024 << "kb for resampling.";

    if( inputDataFloat ) {
        delete [] inputDataFloat;
        inputDataFloat = nullptr;
    }

    if( outputDataFloat ) {
        delete [] outputDataFloat;
        outputDataFloat = nullptr;
    }

    if( outputDataShort ) {
        delete [] outputDataShort;
        outputDataShort = nullptr;
    }

    // Make the input buffer a bit bigger as the amount of data coming in may be a bit more than a frame's worth
    inputDataFloat = new float[( int )( ( double )sampleRate / coreFPS ) * 3];
    outputDataFloat = new float[outputBufferSizeSamples * 2];
    outputDataShort = new short[outputBufferSizeSamples * 2];

}
