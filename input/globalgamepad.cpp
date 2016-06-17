#include "globalgamepad.h"

GlobalGamepad::GlobalGamepad( Node *parent ) : Node( parent ) {    
    NodeAPI::registerNode( this, NodeAPI::Thread::Main, {} );
}
