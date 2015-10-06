#include "videooutput.h"

VideoOutput::VideoOutput( QQuickItem *parent ) : QQuickItem( parent ) {

    // Mandatory for our own drawing code to do anything
    setFlag( QQuickItem::ItemHasContents, true );

}

