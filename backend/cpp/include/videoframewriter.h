#pragma once

#include <QtGlobal>

class VideoFrameWriter {
public:
    virtual ~VideoFrameWriter() {}

    virtual void write( const char *data, quint32 width, quint32 height, size_t pitch ) = 0;
};

