#include "sharedmemory.h"
#include "logging.h"
#include "libretro.h"

#include <QMutex>
#include <QMutexLocker>

static const QString sharedMemoryKey = QStringLiteral( "PHX_FUR_LYFE_BABY!" );

class MemoryLocker {
public:
    MemoryLocker( QSharedMemory *memory )
        : m_memory( memory )
    {
        m_memory->lock();
    }

    ~MemoryLocker() {
        m_memory->unlock();
    }

private:

    QSharedMemory *m_memory;
};
SharedMemory::~SharedMemory() {

}

void SharedMemory::readKeyboardStates(quint8 *t_src, int t_size ) {
    Q_CHECK_PTR( t_src );
    Q_ASSERT( t_size == m_inputBlockSize );

    MemoryLocker memLocker( &m_memory );

    char *memData = static_cast<char *>( m_memory.data() );

    memcpy( t_src, memData + m_inputBlockOffset, t_size );

}


void SharedMemory::writeVideoFrame( uint t_width, uint t_height, uint t_pitch, const void *t_data ) {

    // Construct the total size of the buffer, which will leave up with enough space for the data plus enough
    // size for additional frame metadata, see below.

    // The layout in byte counts is as follows:
    //      [ 1 byte, 4 bytes, 4 bytes, 4 bytes, 4 bytes, height * pitch bytes ]

    // The types of the data are represented like:
    //      [ bool, uint, uint, uint, uint, uchar [] ]

    // The order of the data

    MemoryLocker memLocker( &m_memory );
    (void)memLocker;

    m_videoBlockSize = ( t_height * t_pitch ) + ( sizeof(uint) * 3 );

    Q_ASSERT( m_videoBlockSize > 0 );

    if ( resizeMem() ) {

    }

    char *memData = static_cast<char *>( m_memory.data() );

    if ( t_data ) {

        size_t offset = m_videoBlockOffset;

        memcpy( memData + offset, &t_width, sizeof(t_width) );

        offset += sizeof(t_width);
        memcpy( memData + offset, &t_height, sizeof(t_height) );

        offset += sizeof(t_height);
        memcpy( memData + offset, &t_pitch, sizeof(t_pitch) );

        offset += sizeof(t_pitch);
        memcpy( memData + offset, t_data, t_height * t_pitch );

    }


}

SharedMemory::SharedMemory()
    : m_memory( sharedMemoryKey ),

      m_inputBlockSize( sizeof(quint8) * RETRO_DEVICE_ID_JOYPAD_R3 + 1 ),
      m_videoBlockSize( 0 ),

      m_inputBlockOffset( 0 ),
      m_videoBlockOffset( m_inputBlockOffset + m_inputBlockSize  )

{
    if ( m_memory.isAttached()  ) {
        m_memory.detach();
    }

}

bool SharedMemory::resizeMem() {

    const int newSize = m_inputBlockSize + m_videoBlockSize;

    if ( m_memory.size() == newSize ) {
        return true;
    }

    if ( m_memory.isAttached() ) {
        m_memory.detach();
    }

    m_memory.attach();

    if ( !m_memory.create( newSize ) ) {
        if ( m_memory.error() != QSharedMemory::AlreadyExists ) {
            qCDebug( phxCore )  << "could not allocate memory buffer of size" << newSize
                                << "video is disabled: " << m_memory.errorString();
            return false;
        } else {
            qDebug() << "An old memory allocation will be used. of size:" << m_memory.size();
        }

    } else {
        qDebug() << "Fresh memory was allocated";
    }

    // Zero out the memory;
    for ( int i=0; i < m_memory.size(); ++i ) {
        static_cast<char *>( m_memory.data() )[ i ] = '\0';
    }

    return true;
}
