#include "globalgamepad.h"

GlobalGamepad::GlobalGamepad( Node *parent ) : Node( parent ) {    
    Pipeline::registerNode( this, Pipeline::Thread::Main, {} );
}
