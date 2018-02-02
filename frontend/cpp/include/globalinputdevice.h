#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>

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

class GlobalInputDevice : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool buttonB READ buttonB NOTIFY buttonBChanged)
  Q_PROPERTY(bool buttonY READ buttonY NOTIFY buttonYChanged)
  Q_PROPERTY(bool buttonSelect READ buttonSelect NOTIFY buttonSelectChanged)
  Q_PROPERTY(bool buttonStart READ buttonStart NOTIFY buttonStartChanged)
  Q_PROPERTY(bool buttonUp READ buttonUp NOTIFY buttonUpChanged)
  Q_PROPERTY(bool buttonDown READ buttonDown NOTIFY buttonDownChanged)
  Q_PROPERTY(bool buttonLeft READ buttonLeft NOTIFY buttonLeftChanged)
  Q_PROPERTY(bool buttonRight READ buttonRight NOTIFY buttonRightChanged)
  Q_PROPERTY(bool buttonA READ buttonA NOTIFY buttonAChanged)
  Q_PROPERTY(bool buttonX READ buttonX NOTIFY buttonXChanged)
  Q_PROPERTY(bool buttonL READ buttonL NOTIFY buttonLChanged)
  Q_PROPERTY(bool buttonR READ buttonR NOTIFY buttonRChanged)
  Q_PROPERTY(bool buttonL2 READ buttonL2 NOTIFY buttonL2Changed)
  Q_PROPERTY(bool buttonR2 READ buttonR2 NOTIFY buttonR2Changed)
  Q_PROPERTY(bool buttonL3 READ buttonL3 NOTIFY buttonL3Changed)
  Q_PROPERTY(bool buttonR3 READ buttonR3 NOTIFY buttonR3Changed)

public:
  static GlobalInputDevice &instance();

  bool buttonB() const;
  bool buttonY() const;
  bool buttonSelect() const;
  bool buttonStart() const;
  bool buttonUp() const;
  bool buttonDown() const;
  bool buttonLeft() const;
  bool buttonRight() const;
  bool buttonA() const;
  bool buttonX() const;
  bool buttonL() const;
  bool buttonR() const;
  bool buttonL2() const;
  bool buttonR2() const;
  bool buttonL3() const;
  bool buttonR3() const;

signals:
  void buttonAChanged();
  void buttonBChanged();
  void buttonYChanged();
  void buttonSelectChanged();
  void buttonStartChanged();
  void buttonUpChanged();
  void buttonDownChanged();
  void buttonLeftChanged();
  void buttonRightChanged();
  void buttonXChanged();
  void buttonLChanged();
  void buttonRChanged();
  void buttonL2Changed();
  void buttonR2Changed();
  void buttonL3Changed();
  void buttonR3Changed();

private:
  explicit GlobalInputDevice(QObject* parent = nullptr);

  void setButtonA(int state);
  void setButtonB(int state);
  void setButtonY(int state);
  void setButtonSelect(int state);
  void setButtonStart(int state);
  void setButtonUp(int state);
  void setButtonDown(int state);
  void setButtonLeft(int state);
  void setButtonRight(int state);
  void setButtonX(int state);
  void setButtonL(int state);
  void setButtonR(int state);
  void setButtonL2(int state);
  void setButtonR2(int state);
  void setButtonL3(int state);
  void setButtonR3(int state);

private:
  bool buttonAState{ false };
  bool buttonBState{ false };
  bool buttonYState{ false };
  bool buttonSelectState{ false };
  bool buttonStartState{ false };
  bool buttonUpState{ false };
  bool buttonDownState{ false };
  bool buttonLeftState{ false };
  bool buttonRightState{ false };
  bool buttonXState{ false };
  bool buttonLState{ false };
  bool buttonRState{ false };
  bool buttonL2State{ false };
  bool buttonR2State{ false };
  bool buttonL3State{ false };
  bool buttonR3State{ false };

private slots:
  void onInputStateUpdateChanged(int port, int id, int state);
};
