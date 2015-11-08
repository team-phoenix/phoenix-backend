
#include "audiooutput.h"

const auto samplesPerFrame = 2;
// const auto framesPerSample = 0.5;

AudioOutput::AudioOutput( QObject *parent ): QObject( parent ),
    resamplerState( nullptr ),
    sampleRate( 0 ), hostFPS( 60.0 ), coreFPS( 60.0 ), sampleRateRatio( 1.0 ),
    inputDataFloat( nullptr ),
    outputDataFloat( nullptr ), outputDataShort( nullptr ),
    coreIsRunning( false ),
    outputAudioInterface( nullptr ),
    outputCurrentByte( 0 ),
    outputBuffer( this ),
    outputLengthMs( 64 ), outputTargetMs( 32 ), maxDeviation( 0.005 ) {

    outputBuffer.start();

}

AudioOutput::~AudioOutput() {
    if( outputAudioInterface != nullptr || outputDataFloat != nullptr || outputDataShort != nullptr ) {
        shutdown();
    }
}

//
// Public slots
//

void AudioOutput::consumerFormat( ProducerFormat format ) {
    if( currentState == Control::LOADING ) {
        this->consumerFmt = format;

        this->sampleRate = consumerFmt.audioFormat.sampleRate();
        this->coreFPS = consumerFmt.videoFramerate;
        this->hostFPS = coreFPS;

        qCDebug( phxAudioOutput, "Init audio: %i Hz, %ffps (core), %ffps (host)", sampleRate, coreFPS, hostFPS );

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
        allocateMemory();
    }
}

void AudioOutput::consumerData( QString type, QMutex *mutex, void *data, size_t bytes, qint64 timestamp ) {
    // Buffer pool makes this unnecessary
    Q_UNUSED( mutex )

    if( type == QStringLiteral( "audio" ) ) {

        // Discard data that's too far from the past to matter anymore
        if( QDateTime::currentMSecsSinceEpoch() - timestamp > 500 ) {
            return;
        }

        audioData( ( int16_t * )data, bytes );

    } else if( type == QStringLiteral( "audiovolume" ) ) {

        if( outputAudioInterface ) {
            outputAudioInterface->setVolume( *( qreal * )data );
        }

    }

}

void AudioOutput::setState( Control::State state ) {
    // We only care about the transition to PLAYING, PAUSED or UNLOADING
    if( this->currentState != Control::PLAYING && state == Control::PLAYING ) {
        setAudioActive( true );
    }

    if( this->currentState != Control::PAUSED && state == Control::PAUSED ) {
        setAudioActive( false );
    }

    if( this->currentState != Control::UNLOADING && state == Control::UNLOADING ) {
        setAudioActive( false );
        shutdown();
    }

    this->currentState = state;

}

void AudioOutput::libretroSetFramerate( qreal hostFPS ) {
    qCDebug( phxAudioOutput ).nospace() << "hostFPS = " << hostFPS << "fps";
    this->hostFPS = hostFPS;
}

// Private slots

void AudioOutput::slotAudioOutputStateChanged( QAudio::State s ) {

    if( s == QAudio::IdleState && outputAudioInterface->error() == QAudio::UnderrunError ) {
        // qWarning( phxAudioOutput ) << "audioOut underrun";

        if( outputAudioInterface ) {
            outputAudioInterface->start( &outputBuffer );
        } else {
            resetAudio();
            allocateMemory();
        }
    }

    if( s != QAudio::IdleState && s != QAudio::ActiveState ) {
        qCDebug( phxAudioOutput ) << "State changed:" << s;
    }

}

// Private

void AudioOutput::audioData( int16_t *inputDataShort, int inputBytes ) {

    // Handle the situation where there is an error opening the audio device
    if( outputAudioInterface->error() == QAudio::OpenError ) {
        // qWarning( phxAudioOutput ) << "QAudio::OpenError, attempting reset...";
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

#ifdef DRC_LOGGING

    //    qCDebug( phxAudioOutput ) << "Output is" << ( ( ( double )( ( outputTotalBytes - outputFreeBytes ) ) /
    //                              outputTotalBytes ) * 100 )
    //                              << "% full" << "(target:"
    //                              << ( ( double )outputTargetMs / outputLengthMs ) * 100 << "%)";
    //    qCDebug( phxAudioOutput ) << "DRCRatio =" << DRCRatio
    //                              << "outputAudioInterface->bufferSize()" << outputAudioInterface->bufferSize();
    //    qCDebug( phxAudioOutput ) << "\toutputTotalBytes =" << outputTotalBytes
    //                              << "outputCurrentByte =" << outputCurrentByte
    //                              << " outputFreeBytes =" << outputFreeBytes
    //                              << "inputBytes =" << inputBytes;
    qCDebug( phxAudioOutput ) << "\tOutput buffer is" << outputAudioFormat.durationForBytes( outputTotalBytes ) / 1000
                              << "ms, target =" << outputTargetMs
                              << "ms (" << ( double )100.0 * ( ( double )outputTargetMs / (
                                          ( double )( outputAudioFormat.durationForBytes(
                                                  outputTotalBytes ) ) / 1000.0 ) )
                              << "%)";
    qCDebug( phxAudioOutput ) << "\tOutput: needed" << outputVectorTargetToCurrent << "bytes, wrote"
                              << outputBytesWritten << "bytes";
    qCDebug( phxAudioOutput ) << "\toutputTargetByte =" << outputTargetByte
                              << "outputVectorTargetToCurrent =" << outputVectorTargetToCurrent;
    qCDebug( phxAudioOutput ) << "\toutputAudioInterface->bufferSize() =" << outputAudioInterface->bufferSize()
                              << "outputAudioInterface->bytesFree() =" << outputAudioInterface->bytesFree()
                              << "outputBuffer.bytesToWrite() =" << outputBuffer.bytesToWrite();
    qCDebug( phxAudioOutput ) << "\toutputBuffer.bytesAvailable() =" << outputBuffer.bytesAvailable()
                              << "outputBuffer.size() =" << outputBuffer.size()
                              << "outputBuffer.pos() =" << outputBuffer.pos();
    qCDebug( phxAudioOutput ) << "\tState:" << outputAudioInterface->state()
                              << " error: " << outputAudioInterface->error();

#endif

}

void AudioOutput::setAudioActive( bool coreIsRunning ) {

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

void AudioOutput::shutdown() {

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

    outputCurrentByte = 0;
    outputBuffer.clear();

    qCDebug( phxAudioOutput ) << "slotShutdown() end";

}

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
    // QAudio::State state = outputAudioInterface->state();
    // qCDebug( phxAudioOutput ) << state;

    if( !coreIsRunning ) {
        outputAudioInterface->suspend();
    }
}

void AudioOutput::allocateMemory() {

    // Some cores may give as much as 4 video frames' worth of audio data in a single video frame period, so we need to one-up them
    int bufferSizeInVideoFrames = 30;

    auto outputBufferSizeSamples = outputAudioFormat.bytesForDuration( outputLengthMs * 1000 );
    qCDebug( phxAudioOutput ) << "Allocating" <<
                              ( float )(
                                  sizeof( float ) * ( int )( ( double )sampleRate / coreFPS ) * bufferSizeInVideoFrames
                                  + sizeof( float ) * outputBufferSizeSamples * bufferSizeInVideoFrames
                                  + sizeof( short ) * outputBufferSizeSamples * bufferSizeInVideoFrames
                              )
                              / 1024.0 / 1024.0 << "MB for resampling...";

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
    inputDataFloat = new float[( int )( ( double )sampleRate / coreFPS ) * bufferSizeInVideoFrames];
    outputDataFloat = new float[outputBufferSizeSamples * bufferSizeInVideoFrames];
    outputDataShort = new short[outputBufferSizeSamples * bufferSizeInVideoFrames];
}

