#include "inputdevice.h"

/*

InputDevice::InputDevice( int port, GamepadType type )
    : mDigitalStates( static_cast<int>( DigitalButtons::Last ) + 1, 0 ),
      mAnalogStates( static_cast<int>( AnalogButtons::Last ) + 1, 0 ),
      mPort( port ),
      mType( type )
{
    Q_ASSERT( mDigitalStates.size() == ( static_cast<int>( DigitalButtons::Last ) + 1 ) );
    Q_ASSERT( mAnalogStates.size() == ( static_cast<int>( AnalogButtons::Last ) + 1 ) );
}

void InputDevice::remapInput(int key, const InputMapValue &value) {
    mInputMap[ key ] = value;
}

void InputDevice::remapInput(int key, InputMapValue &&value) {
    remapInput( key, value );
}

void InputDevice::remapInput(const InputDevice::InputMap &map) {
    mInputMap = map;
}

void InputDevice::remapInput(InputDevice::InputMap &&map) {
    remapInput( map );
}

int InputDevice::getDigitalState(int type) {
    return mDigitalStates.value( type, 0 );
}

int InputDevice::getAnalogState(int type) {
    return mAnalogStates.value( type, 0 );
}

const QString InputDevice::name() const {
    return mName;
}

QString InputDevice::mappingString() const {
    return qmlMappingString;
}

GamepadType InputDevice::type() const {
    return mType;
}

int InputDevice::port() const {
    return mPort;
}

void InputDevice::setName( const QString name ) {
    mName = name;
}
*/
