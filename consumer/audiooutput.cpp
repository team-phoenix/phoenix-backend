#include "audiooutput.h"

// FIXME: Stop assuming stereo?
const auto samplesPerFrame = 2;
// const auto framesPerSample = 0.5;

AudioOutput::AudioOutput( Node *parent ): Node( parent ),
    outputBuffer( this ) {

    outputBuffer.start();
}

AudioOutput::~AudioOutput() {
    if( outputAudioInterface != nullptr || outputDataFloat != nullptr || outputDataShort != nullptr ) {
        shutdown();
    }
}

// Public slots

void AudioOutput::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    Node::commandIn( command, data, timeStamp );

    switch( command ) {
        case Command::Play: {
            qCDebug( phxAudioOutput ) << command;
            state = State::Playing;
            pipelineStateChanged();
            break;
        }

        case Command::Stop: {
            qCDebug( phxAudioOutput ) << command;
            state = State::Stopped;
            pipelineStateChanged();
            break;
        }

        case Command::Load: {
            qCDebug( phxAudioOutput ) << command;
            state = State::Loading;
            pipelineStateChanged();
            break;
        }

        case Command::Pause: {
            qCDebug( phxAudioOutput ) << command;
            state = State::Paused;
            pipelineStateChanged();
            break;
        }

        case Command::Unload: {
            qCDebug( phxAudioOutput ) << command;
            state = State::Unloading;
            pipelineStateChanged();
            shutdown();
        }

        // For debugging. Prints number of heartbeats it gets each second (it can only check how much time has passed
        // each time it gets a heartbeat)
        case Command::Heartbeat: {
            static int counter = 0;
            static qint64 secondLater = QDateTime::currentMSecsSinceEpoch() + 1000;

            counter++;

            if( timeStamp > secondLater ) {
                //qCDebug( phxAudioOutput ) << counter;
                counter = 0;
                secondLater = timeStamp + 1000;
            }

            break;
        }

        case Command::HostFPS: {
            hostFPS = data.toReal();
            qCDebug( phxAudioOutput ).nospace() << "hostFPS = " << hostFPS << "fps, vsync: " << vsync;
            break;
        }

        case Command::CoreFPS: {
            coreFPS = data.toReal();
            qCDebug( phxAudioOutput ).nospace() << "coreFPS = " << coreFPS << "fps, vsync: " << vsync;
            break;
        }

        case Command::AudioFormat: {
            // Currently, we assume that the audio format will only be set once per session, during loading
            if( state == State::Loading ) {
                this->format = qvariant_cast<ProducerFormat>( data );

                this->sampleRate = format.audioFormat.sampleRate();
                this->coreFPS = format.videoFramerate;
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

                resetAudio( );
                allocateMemory();

                outputLengthMs = outputAudioFormat.durationForBytes( outputAudioInterface->bufferSize() ) / 1000;
            }

            break;
        }

        case Command::SetVolume: {
            if( outputAudioInterface ) {
                outputAudioInterface->setVolume( data.toReal() );
            }

            break;
        }

        case Command::SetVsync: {
            vsync = data.toBool();
            qCDebug( phxAudioOutput ).nospace() << "vsync: " << vsync;
        }

        default:
            break;
    }
}

void AudioOutput::dataIn( Node::DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) {
    if( type == DataType::Audio ) {
        if( state == State::Playing ) {
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

            // Discard data that's too far from the past to matter anymore
            if( currentTime - timeStamp > 500 ) {
                static qint64 lastMessage = QDateTime::currentMSecsSinceEpoch();

                if( currentTime - lastMessage > 1000 ) {
                    lastMessage = currentTime;
                    // qCWarning( phxAudioOutput ) << "Discarding" << bytes << "bytes of old audio data from" <<
                    //   currentTime - timestamp << "ms ago";
                }

                return;
            }

            // Make a copy so the data won't be changed later
            mutex->lock();
            memcpy( inputDataShort, data, bytes );
            mutex->unlock();

            // Handle the situation where there is an error opening the audio device
            if( outputAudioInterface->error() == QAudio::OpenError ) {
                // qWarning( phxAudioOutput ) << "QAudio::OpenError, attempting reset...";
                resetAudio();
            }

            int inputBytes = bytes;
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
            // The vector is negative if beyond the target and positive if behind
            int outputTargetByte = outputAudioFormat.bytesForDuration( outputTargetMs * 1000 );
            int outputEstimatedBytes = outputAudioFormat.bytesForDuration( inputAudioFormat.durationForBytes( inputBytes ) );
            int outputVectorTargetToCurrent = outputTargetByte - outputCurrentByte + outputEstimatedBytes;
            // double unclampedDRCScale = ( double )outputVectorTargetToCurrent / outputTargetByte;
            double unclampedDRCScale = ( double )outputVectorTargetToCurrent + outputEstimatedBytes / outputEstimatedBytes;

            // Calculate the final DRC ratio
            double DRCScale = qMax( -maxDeviation, qMin( unclampedDRCScale, maxDeviation ) );
            double hostRatio = vsync ? hostFPS / coreFPS : 1.0;
            double adjustedSampleRateRatio = sampleRateRatio * ( 1.0 + DRCScale ) * hostRatio;

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
            outputCurrentByte += outputBytesWritten;

#if defined( DRC_LOGGING )
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            static qint64 lastMessage = 0;

            qint64 inputBytesPerMSec = inputAudioFormat.bytesForDuration( 1000 );
            qint64 outputBytesPerMSec = outputAudioFormat.bytesForDuration( 1000 );

            if( currentTime - lastMessage > 1000 ) {
                lastMessage = currentTime;
                qCDebug( phxAudioOutput ) << "Input:" << inputBytes / inputBytesPerMSec << "ms";
                qCDebug( phxAudioOutput ) << "Output is" << ( ( ( double )( ( outputTotalBytes - outputFreeBytes ) ) /
                                          outputTotalBytes ) * 100 )
                                          << "% full," << ( outputTotalBytes - outputFreeBytes ) / outputBytesPerMSec << "ms (target:" << outputTargetMs << "ms /"
                                          << ( ( double )outputTargetMs / outputLengthMs ) * 100 << "%)";
                qCDebug( phxAudioOutput ) << "\tunclampedDRCScale =" << unclampedDRCScale << "DRCScale =" << DRCScale
                                          << "outputAudioInterface->bufferSize()" << outputAudioInterface->bufferSize() / outputBytesPerMSec << "ms";
                qCDebug( phxAudioOutput ) << "\toutputTotalBytes =" << outputTotalBytes / outputBytesPerMSec << "ms"
                                          << "outputCurrentByte =" << outputCurrentByte / outputBytesPerMSec << "ms"
                                          << " outputFreeBytes =" << outputFreeBytes / outputBytesPerMSec << "ms";
                qCDebug( phxAudioOutput ) << "\tOutput buffer is" << outputLengthMs
                                          << "ms, target =" << outputTargetMs
                                          << "ms (" << ( double )100.0 * ( ( double )outputTargetMs / (
                                                      ( double )( outputAudioFormat.durationForBytes(
                                                              outputTotalBytes ) ) / 1000.0 ) )
                                          << "%)";
                qCDebug( phxAudioOutput ) << "\tOutput: needed" << outputVectorTargetToCurrent / outputBytesPerMSec << "ms, wrote"
                                          << outputBytesWritten / outputBytesPerMSec << "ms";
                qCDebug( phxAudioOutput ) << "\toutputTargetByte =" << outputTargetByte / outputBytesPerMSec << "ms"
                                          << "outputVectorTargetToCurrent =" << outputVectorTargetToCurrent / outputBytesPerMSec << "ms";
                qCDebug( phxAudioOutput ) << "\toutputAudioInterface->bufferSize() =" << outputAudioInterface->bufferSize() / outputBytesPerMSec << "ms"
                                          << "outputAudioInterface->bytesFree() =" << outputAudioInterface->bytesFree() / outputBytesPerMSec << "ms"
                                          << "outputBuffer.bytesToWrite() =" << outputBuffer.bytesToWrite() / outputBytesPerMSec << "ms";
                qCDebug( phxAudioOutput ) << "\toutputBuffer.bytesAvailable() =" << outputBuffer.bytesAvailable() / outputBytesPerMSec << "ms"
                                          << "outputBuffer.size() =" << outputBuffer.size() / outputBytesPerMSec << "ms"
                                          << "outputBuffer.pos() =" << outputBuffer.pos() / outputBytesPerMSec << "ms";
                // qCDebug( phxAudioOutput ) << "\tState:" << outputAudioInterface->state()
                //   << " error: " << outputAudioInterface->error();
            }

#endif
        }
    }

    Node::dataIn( type, mutex, data, bytes, timeStamp );
}

// Private slots

void AudioOutput::outputStateChanged( QAudio::State outputState ) {
    if( outputState == QAudio::IdleState && outputAudioInterface->error() == QAudio::UnderrunError ) {
        // Schedule the audio to be reset at some point in the future, giving the buffer some time to fill
        QTimer::singleShot( 50, this, &AudioOutput::outputUnderflow );
    }

    if( outputState == QAudio::SuspendedState ) {
        qCDebug( phxAudioOutput ) << "Output state changed:" << outputState;
    }
}

void AudioOutput::outputUnderflow() {
    if( outputAudioInterface && state == State::Playing ) {
        // qCWarning( phxAudioOutput ) << "Audio buffer underflow";
        outputAudioInterface->start( &outputBuffer );
    }

    else if( !outputAudioInterface ) {
        qCWarning( phxAudioOutput ) << "Audio buffer underflow (resetting)";
        resetAudio();
        allocateMemory();
    }
}

// Private

void AudioOutput::pipelineStateChanged() {
    if( !outputAudioInterface ) {
        return;
    }

    if( !( state == State::Playing ) ) {
        if( outputAudioInterface->state() != QAudio::SuspendedState ) {
            qCDebug( phxAudioOutput ) << "Paused";
            outputAudioInterface->suspend();
        }
    }

    else {
        if( outputAudioInterface->state() != QAudio::ActiveState ) {
            qCDebug( phxAudioOutput ) << "Started";
            outputAudioInterface->resume();
        }
    }
}

void AudioOutput::shutdown() {
    qCDebug( phxAudioOutput ) << "shutdown() start";

    if( outputAudioInterface ) {
        outputAudioInterface->stop();
        delete outputAudioInterface;
        outputAudioInterface = nullptr;
    }

    if( inputDataShort ) {
        delete [] inputDataShort;
        inputDataShort = nullptr;
    }

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

    outputCurrentByte = 0;
    outputBuffer.clear();

    qCDebug( phxAudioOutput ) << "shutdown() end";
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

    connect( outputAudioInterface, &QAudioOutput::stateChanged, this, &AudioOutput::outputStateChanged );
    outputAudioInterface->start( &outputBuffer );
    // QAudio::State state = outputAudioInterface->state();
    // qCDebug( phxAudioOutput ) << state;

    if( !( state == State::Playing ) ) {
        outputAudioInterface->suspend();
    }
}

void AudioOutput::allocateMemory() {
    // Some cores may give as much as 4 video frames' worth of audio data in a single video frame period, so we need to one-up them
    const static int bufferSizeInVideoFrames = 30;
    size_t inputBufferSamples = samplesPerFrame * inputAudioFormat.framesForDuration( ( 1000 / coreFPS ) * bufferSizeInVideoFrames * 1000 );
    size_t outputBufferSamples = samplesPerFrame * outputAudioFormat.framesForDuration( outputLengthMs * 1000 );

    qCDebug( phxAudioOutput ) << "Allocating" <<
                              ( double )(
                                  sizeof( float ) * ( inputBufferSamples + outputBufferSamples ) +
                                  sizeof( short ) * ( inputBufferSamples + outputBufferSamples )
                              )
                              / 1024.0 << "KB for resampling";
    qCDebug( phxAudioOutput ).nospace() << "Input buffer samples: " << inputBufferSamples <<
                                        ", output buffer samples: " << outputBufferSamples;

    if( inputDataShort ) {
        delete [] inputDataShort;
        inputDataShort = nullptr;
    }

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

    inputDataShort = new short[ inputBufferSamples ]();
    inputDataFloat = new float[ inputBufferSamples ]();
    outputDataFloat = new float[ outputBufferSamples ]();
    outputDataShort = new short[ outputBufferSamples ]();
}

