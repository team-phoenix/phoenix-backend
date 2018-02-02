#include "globalinputdevice.h"
#include "emulationlistener.h"

#define RETRO_DEVICE_ID_JOYPAD_B        0
#define RETRO_DEVICE_ID_JOYPAD_Y        1
#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
#define RETRO_DEVICE_ID_JOYPAD_START    3
#define RETRO_DEVICE_ID_JOYPAD_UP       4
#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
#define RETRO_DEVICE_ID_JOYPAD_A        8
#define RETRO_DEVICE_ID_JOYPAD_X        9
#define RETRO_DEVICE_ID_JOYPAD_L       10
#define RETRO_DEVICE_ID_JOYPAD_R       11
#define RETRO_DEVICE_ID_JOYPAD_L2      12
#define RETRO_DEVICE_ID_JOYPAD_R2      13
#define RETRO_DEVICE_ID_JOYPAD_L3      14
#define RETRO_DEVICE_ID_JOYPAD_R3      15

GlobalInputDevice &GlobalInputDevice::instance()
{
  static GlobalInputDevice globalInputDevice;
  return globalInputDevice;
}

bool GlobalInputDevice::buttonA() const
{
  return buttonAState;
}

bool GlobalInputDevice::buttonX() const {return buttonXState;}

bool GlobalInputDevice::buttonL() const {return buttonLState;}

bool GlobalInputDevice::buttonR() const {return buttonRState;}

bool GlobalInputDevice::buttonL2() const {return buttonL2State;}

bool GlobalInputDevice::buttonR2() const {return buttonR2State;}

bool GlobalInputDevice::buttonL3() const {return buttonL3State;}

bool GlobalInputDevice::buttonR3() const {return buttonR3State;}

bool GlobalInputDevice::buttonB() const { return buttonBState; }

bool GlobalInputDevice::buttonY() const { return buttonYState;}

bool GlobalInputDevice::buttonSelect() const {return buttonSelectState;}

bool GlobalInputDevice::buttonStart() const {return buttonStartState;}

bool GlobalInputDevice::buttonUp() const {return buttonUpState;}

bool GlobalInputDevice::buttonDown() const {return buttonDownState;}

bool GlobalInputDevice::buttonLeft() const {return buttonLeftState;}

bool GlobalInputDevice::buttonRight() const {return buttonRightState;}

GlobalInputDevice::GlobalInputDevice(QObject* parent)
  : QObject(parent)
{

  connect(&EmulationListener::instance(), &EmulationListener::inputStateUpdated, this,
          &GlobalInputDevice::onInputStateUpdateChanged);
}

void GlobalInputDevice::setButtonA(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonAState) {
    buttonAState = buttonState;
    emit buttonAChanged();
  }
}

void GlobalInputDevice::setButtonB(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonBState) {
    buttonBState = buttonState;
    emit buttonBChanged();
  }
}

void GlobalInputDevice::setButtonY(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonYState) {
    buttonYState = buttonState;
    emit buttonYChanged();
  }
}

void GlobalInputDevice::setButtonSelect(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonSelectState) {
    buttonSelectState = buttonState;
    emit buttonSelectChanged();
  }
}

void GlobalInputDevice::setButtonStart(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonStartState) {
    buttonStartState = buttonState;
    emit buttonStartChanged();
  }
}

void GlobalInputDevice::setButtonUp(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonUpState) {
    buttonUpState = buttonState;
    emit buttonUpChanged();
  }
}

void GlobalInputDevice::setButtonDown(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonDownState) {
    buttonDownState = buttonState;
    emit buttonDownChanged();
  }
}

void GlobalInputDevice::setButtonLeft(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonLeftState) {
    buttonLeftState = buttonState;
    emit buttonLeftChanged();
  }
}

void GlobalInputDevice::setButtonRight(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonRightState) {
    buttonRightState = buttonState;
    emit buttonRightChanged();
  }
}

void GlobalInputDevice::setButtonX(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonXState) {
    buttonXState = buttonState;
    emit buttonXChanged();
  }
}

void GlobalInputDevice::setButtonL(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonLState) {
    buttonLState = buttonState;
    emit buttonLChanged();
  }
}

void GlobalInputDevice::setButtonR(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonRState) {
    buttonRState = buttonState;
    emit buttonRChanged();
  }
}

void GlobalInputDevice::setButtonL2(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonL2State) {
    buttonL2State = buttonState;
    emit buttonL2Changed();
  }
}

void GlobalInputDevice::setButtonR2(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonR2State) {
    buttonR2State = buttonState;
    emit buttonR2Changed();
  }
}

void GlobalInputDevice::setButtonL3(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonL3State) {
    buttonL3State = buttonState;
    emit buttonL3Changed();
  }
}

void GlobalInputDevice::setButtonR3(int state)
{
  bool buttonState = (state == 1);

  if (buttonState != buttonR3State) {
    buttonR3State = buttonState;
    emit buttonR3Changed();
  }
}

void GlobalInputDevice::onInputStateUpdateChanged(int port, int id, int state)
{
  Q_UNUSED(port);

  switch (id) {
    case RETRO_DEVICE_ID_JOYPAD_B:
      setButtonB(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_Y:
      setButtonY(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_SELECT:
      setButtonSelect(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_START:
      setButtonStart(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_UP:
      setButtonUp(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_DOWN:
      setButtonDown(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_LEFT:
      setButtonLeft(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_A:
      setButtonA(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_X:
      setButtonX(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_L:
      setButtonL(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_R:
      setButtonR(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_L2:
      setButtonL2(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_R2:
      setButtonR2(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_L3:
      setButtonL3(state);
      break;

    case RETRO_DEVICE_ID_JOYPAD_R3:
      setButtonR3(state);
      break;

    default:
      break;
  }

//  qDebug() << port << id << state;
}

