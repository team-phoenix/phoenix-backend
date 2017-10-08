#pragma once

#include "sharedmemory.h"
#include "emulator.h"

#include <QObject>
#include <QMutex>

static QMutex memoryMutex;

class Test_SharedMemory : public QObject {
    Q_OBJECT
    SharedMemory *memory;
    char *rawMemoryBuffer;

    template<typename T, typename R>
    T read( R *src, size_t &offset ) {
        const size_t len = sizeof(T) / sizeof( char );
        char array[ len ];
        for ( size_t i=0; i < len; ++i ) {
            array[ i ] = static_cast<char *>( src )[ offset ];
            offset += 1;
        }

        return *reinterpret_cast<T *>( array );
    }

    void writeSomeVideoDataToMemory( uint t_width = 10, uint t_height = 10, char payload = 1 ) {
        const uint rbgPixelSize = 3;
        const uint width = t_width;
        const uint height = t_height;
        const uint pitch = width * sizeof( rbgPixelSize );

        const int totalBufferSize = ( height * pitch ) + ( sizeof(uint) * rbgPixelSize );

        char buffer[totalBufferSize];

        for ( int i=0; i < totalBufferSize; ++i ) {
            buffer[ i ] = payload;
        }

        memory->writeVideoFrame( width, height, pitch, buffer );
    }

private slots:

    void initTestCase() {
        // The Emulator instance must be removed since keeps the shared memory alive.
        delete Emulator::instance();
    }

    void init() {
        //memoryMutex.lock();
        memory = new SharedMemory;
        rawMemoryBuffer = static_cast<char *>( memory->m_memory.data() );
    }

    void cleanup() {
        delete memory;
        //memoryMutex.unlock();
    }

    void should_readKeyboardStatesFromMemoryAndCopyToArray() {

        const int size = memory->m_inputBlockSize;
        quint8 states[ size ];
        quint8 originalStates[ size ];

        for ( int i=0; i < size; ++i ) {
            states[ i ] = 3;
            originalStates[ i ] = states[ i ];
        }

        memory->readKeyboardStates( states, size ) ;

        for ( int i=0; i < size; ++i ) {
            QVERIFY( states[ i ] != originalStates[ i ] );
        }
    }

    void should_resizeSharedMemoryBlockShouldAttachedOnCreation() {
        QCOMPARE( memory->m_memory.isAttached(), true );
    }

    void should_zeroOutSharedMemoryDuringResize() {

        for ( int i=0; i < memory->m_memory.size(); ++i ) {
            QCOMPARE( static_cast<char *>( memory->m_memory.data() )[ i ], '\0' );
        }
    }

    void should_haveSharedMemoryBufferSizeEqualOrGreaterThanSuppliedSize() {
        QVERIFY( memory->m_memory.size() >= static_cast<int>( memory->m_inputBlockSize + memory->m_videoBlockSize ) );
    }

    void should_beAbletoWriteVideoDataToMemory() {
        writeSomeVideoDataToMemory();
    }

    void should_haveVideoOffsetEqualTo16Bytes() {
        QCOMPARE( static_cast<int>( memory->m_videoBlockOffset ), 16 );
    }

    void should_verifyThatStartingBufferSizeIsEqualToInputBlockSize() {
        QCOMPARE( memory->m_memory.size(), static_cast<int>( memory->m_inputBlockSize ) );
    }

    void should_resizeMemoryToADifferentSize() {

        const int oldMemorySize = memory->m_memory.size();

        memory->m_videoBlockSize += 10;
        memory->resizeMem();

        QVERIFY( oldMemorySize != memory->m_memory.size() );
    }

    void should_verifyThatUpdateFlagIsAlwaysSetProperly() {

        memory->m_videoBlockSize = 10;
        memory->resizeMem();

        size_t offset = memory->m_videoBlockOffset;
        rawMemoryBuffer = static_cast<char *>( memory->m_memory.data() );

        bool updateFlag = read<bool>( rawMemoryBuffer, offset );

        QCOMPARE( updateFlag, false );

        writeSomeVideoDataToMemory( 20, 20, 2 );

        size_t originalOffset = 16;
        updateFlag = read<bool>( rawMemoryBuffer, originalOffset );

        QCOMPARE( updateFlag, true );
    }

    void should_verifyThatWrittenVideoDataIsInMemory() {

        writeSomeVideoDataToMemory( 20, 20, 2 );

        QVERIFY( rawMemoryBuffer != nullptr );

        size_t offset = 16;

        const bool updateFlag = read<bool>( rawMemoryBuffer, offset );

        QCOMPARE( updateFlag, true );


//        for ( int i=0; i < totalBufferSize; ++i ) {
//            qDebug() << i;
//            QCOMPARE( storeReadBuffer[ i ], static_cast<char>( 1 ) );
//        }

    }

    /*
     * size_t offset = 16;

    Q_CHECK_PTR( m_sharedVideoMem->data() );

    char *memory = static_cast<char *>( m_sharedVideoMem->data() );

    const bool updateFlag = read<bool>( memory, offset );

    if ( updateFlag ) {

        unsigned vidWidth = read<unsigned>(memory, offset );

        unsigned vidHeight = read<unsigned>( memory, offset );

        unsigned vidPitch = read<unsigned>( memory, offset );

        const uchar *vidBytes = reinterpret_cast<uchar *>( memory ) + offset;

        if ( static_cast<uint>( m_videoBuffer.size() ) != vidHeight * vidPitch ) {
            m_videoBuffer.resize( vidHeight * vidPitch );
        }

        for ( int i=0; i < m_videoBuffer.size(); ++i ) {
            m_videoBuffer[ i ] = vidBytes[ i ];
        }

        m_videoTexture = QImage( m_videoBuffer.data()
                                 , vidWidth, vidHeight, vidPitch
                                 , static_cast<QImage::Format>( m_pixelFormat ) );

        if ( m_videoTexture.isNull() ) {
            qDebug() << "video texture is null";
            Q_ASSERT( false );
        } else {
            update();
        }

        offset = 16;
        static const bool droppedFrame = false;
        memcpy( memory + offset, &droppedFrame, sizeof( droppedFrame ) );

    }
     */
};
