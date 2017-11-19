
#include "libretro.h"

#include <QVector>

template<typename KeyboardStateBuffer = QVector<qint16>>
class InputManager_T
{
public:

  InputManager_T()
  {
    keyboardStates.resize( RETRO_DEVICE_ID_JOYPAD_R3 + 1);
    keyboardStates.fill( 0x0 );
  }

  qint16* getKeyboardBuffer()
  {
    return keyboardStates.data();
  }

  int getKeyboardBufferSize() const
  {
    return keyboardStates.size();
  }

  void clear()
  {
    keyboardStates.clear();
  }

private:
  KeyboardStateBuffer keyboardStates;
};

using InputManager = InputManager_T<>;
