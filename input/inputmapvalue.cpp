#include "inputmapvalue.h"

#include <QHash>

using namespace Input;

uint qHash(const InputMapValue &value, uint seed) {
    return qHash( static_cast<uint>( value.key ), seed );
}
