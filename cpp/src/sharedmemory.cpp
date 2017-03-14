#include "sharedmemory.h"
#include "logging.h"

#include <QMutex>
#include <QMutexLocker>

/*
 *
 * [  header = [ input, video ]
 *
 *
 */

static const QString sharedMemoryKey = QStringLiteral( "PHX_FUR_LYFE_BABY!" );
const int defaultInputBlockSize = sizeof(quint8) + sizeof(quint8);

class MemoryLocker {
public:
    MemoryLocker( SharedMemory *memory )
        : m_memory( memory )
    {
        m_memory->lock();
    }

    ~MemoryLocker() {
        m_memory->unlock();
    }

private:
    SharedMemory *m_memory;
};

SharedMemory::~SharedMemory() {
    delete m_mutex;
    m_mutex = nullptr;
}

void SharedMemory::setVideoMemory( uint t_width, uint t_height, uint t_pitch, const void *t_data ) {
    QMutexLocker locker( m_mutex );

    // Construct the total size of the buffer, which will leave up with enough space for the data plus enough
    // size for additional frame metadata, see below.

    // The layout in byte counts is as follows:
    //      [ 1 byte, 4 bytes, 4 bytes, 4 bytes, 4 bytes, height * pitch bytes ]

    // The types of the data are represented like:
    //      [ bool, uint, uint, uint, uint, uchar [] ]

    // The order of the data

    int blockSize = ( t_height * t_pitch ) + ( sizeof(uint) * 3 );

    if ( resizeMem( blockSize ) ) {
    }

    char *memData = static_cast<char *>( data() );

    if ( t_data ) {

        MemoryLocker memLocker( this );

        size_t offset = 0;

        memcpy( memData + offset, &t_width, sizeof(t_width) );

        offset += sizeof(t_width);
        memcpy( memData + offset, &t_height, sizeof(t_height) );

        offset += sizeof(t_height);
        memcpy( memData + offset, &t_pitch, sizeof(t_pitch) );

        offset += sizeof(t_pitch);
        memcpy( memData + offset, t_data, t_height * t_pitch );

    }


}

SharedMemory &SharedMemory::instance() {
    static SharedMemory memory;
    return memory;
}

SharedMemory::SharedMemory(QObject *parent)
    : QSharedMemory( sharedMemoryKey, parent ),
      m_mutex( new QMutex ),
      m_videoBlockOffset( 0 ),
      m_inputBlockOffset( 0 )
{
    if ( isAttached()  ) {
        detach();
    }

}

bool SharedMemory::resizeMem( int t_blockSize ) {

    if ( size() == t_blockSize ) {
        return true;
    }

    if ( isAttached() ) {
        detach();
    }

    attach();

    if ( !create( t_blockSize ) ) {
        if ( error() != QSharedMemory::AlreadyExists ) {
            qCDebug( phxCore )  << "could not allocate memory buffer of size" << t_blockSize
                                << "video is disabled: " << errorString();
            return false;
        } else {
            qDebug() << "An old memory allocation will be used. of size:" << size();
        }

    } else {
        qDebug() << "Fresh memory was allocated";
    }

    // Zero out the memory;
    for ( int i=0; i < size(); ++i ) {
        static_cast<char *>( data() )[ i ] = '\0';
    }

    return true;
}
