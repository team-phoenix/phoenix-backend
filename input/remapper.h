#pragma once

#include <QMutex>
#include <QObject>
#include <QHash>
#include <QMap>
#include <Qt>

#include "SDL.h"
#include "SDL_gamecontroller.h"

#include "node.h"
#include "gamepadstate.h"
#include "keyboardstate.h"
#include "logging.h"

typedef QMap<QString, QString> QStringMap;

/*
 * Remapper is a Node whose job is to transform input data that passes through it based on stored remapping data.
 * It also provides signals and slots to communicate with a QML-based frontend that will present the user with a
 * graphical interface for setting remappings.
 *
 * The remapper supports the button layout of an Xbox 360 controller (these are the actual strings used):
 *  Face buttons: A B X Y
 *  Center buttons: Back Guide Start
 *  DPAD: Up Down Left Right
 *  Bumpers: L R
 *  Stick buttons: L3 R3
 *
 * Note that L2 and R2 are not directly remappable, they're analog on the Xbox 360 controller (TODO?)
 *
 * The remapper also lets the user remap the keyboard. (TODO)
 */

class RemapperModel;

class Remapper : public Node {
        Q_OBJECT

    public:
        explicit Remapper( Node *parent = nullptr );

    signals:
        // Signals for RemapperModel

        // A new controller GUID was seen, add to the model's list
        void controllerAdded( QString GUID, QString friendlyName );

        // The last remaining controller with this GUID was removed, do not accept remap requests for this one
        void controllerRemoved( QString GUID );

        // Only fired while not in remap mode, true if any button on any controller with this GUID has been pressed
        void buttonUpdate( QString GUID, bool pressed );

        // Remap mode completed, update the UI
        void remapModeEnd();

        // Data for the model so it can update its internal copy of remapData
        void remapUpdate( QString GUID, QString originalButton, QString remappedButton );

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

        void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) override;

        // Do not send input updates to this node's children, listen for a button press on this GUID and reply back
        // with a button once pressed
        void remapModeBegin( QString GUID, QString button );

    private:
        // Node stuff

        // True iff we're playing right now
        bool playing{ false };

        // Producer stuff

        // If true, do not send any data to children
        bool remapMode{ false };

        // The GUID being remapped
        QString remapModeGUID;

        // The button ID of the GUID getting the remap
        int remapModeButton{ SDL_CONTROLLER_BUTTON_INVALID };

        // If set to true, remain true until ignoreModeButton is released
        bool ignoreMode{ false };

        // The button pressed to set the remapping
        int ignoreModeButton{ SDL_CONTROLLER_BUTTON_INVALID };

        // The GUID that was just remapped
        QString ignoreModeGUID;

        // The instanceID that was just remapped
        int ignoreModeInstanceID{ -1 };

        // Ensure reads and writes to gamepadBuffer are atomic
        QMutex mutex;

        // A circular buffer that holds gamepad state updates
        // A value of 100 should be sufficient for most purposes
        GamepadState gamepadBuffer[ 100 ];
        int gamepadBufferIndex{ 0 };

        // Used to track which GUIDs have a button pressed
        // Cleared on each heartbeat
        QMap<QString, bool> pressed;

        // True if a mapped keyboard key is pressed
        bool keyboardKeyPressed { false };

        // Remapping stuff

        // Remap data
        QMap<QString, QMap<int, int>> gamepadSDLButtonToSDLButton;

        // Convert analog stick values to d-pad presses and vice versa
        // This is useful to both LibretroCore and the UI (GlobalGamepad)
        // analogToDpad is always active when not playing
        QMap<QString, bool> analogToDpad;
        QMap<QString, bool> dpadToAnalog;

        bool dpadToAnalogKeyboard { false };

        // Keyboard remapping data
        // We keep the keyboard's state here as it comes to us "raw"
        GamepadState keyboardGamepad;

        // Default keyboard mapping
        QMap<int, int> keyboardKeyToSDLButton {
            { Qt::Key_W, SDL_CONTROLLER_BUTTON_DPAD_UP },
            { Qt::Key_A, SDL_CONTROLLER_BUTTON_DPAD_LEFT },
            { Qt::Key_S, SDL_CONTROLLER_BUTTON_DPAD_DOWN },
            { Qt::Key_D, SDL_CONTROLLER_BUTTON_DPAD_RIGHT },
            { Qt::Key_P, SDL_CONTROLLER_BUTTON_A },
            { Qt::Key_L, SDL_CONTROLLER_BUTTON_B },
            { Qt::Key_O, SDL_CONTROLLER_BUTTON_X },
            { Qt::Key_K, SDL_CONTROLLER_BUTTON_Y },
            { Qt::Key_Shift, SDL_CONTROLLER_BUTTON_LEFTSHOULDER },
            { Qt::Key_Space, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
            { Qt::Key_Backspace, SDL_CONTROLLER_BUTTON_BACK },
            { Qt::Key_Escape, SDL_CONTROLLER_BUTTON_GUIDE },
            { Qt::Key_Return, SDL_CONTROLLER_BUTTON_START },
            { Qt::Key_1, SDL_CONTROLLER_BUTTON_LEFTSTICK },
            { Qt::Key_3, SDL_CONTROLLER_BUTTON_RIGHTSTICK },
        };

        // Internal bookkeeping

        // Count of how many controllers of a certain GUID are connected
        // When 0, the entry may be removed
        // Used to know when it's time to emit controllerConnected()/controllerDisconnected()
        QMap<QString, int> GUIDCount;

        // Helpers

        // If clear is set, analog state is cleared if no d-pad buttons pressed
        GamepadState mapDpadToAnalog( GamepadState gamepad, bool clear = false );

        QString buttonToString( int button );

        int stringToButton( QString button );
};
