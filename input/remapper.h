#pragma once

#include <QMutex>
#include <QObject>
#include <QHash>
#include <QMap>

#include "SDL.h"
#include "SDL_gamecontroller.h"

#include "node.h"
#include "gamepad.h"
#include "logging.h"

typedef QMap<QString, QString> QStringMap;

/*
 * Remapper is a Node whose job is to filter and transform input data that passes through it based on stored remapping
 * data. It also provides signals and slots to communicate with a QML-based frontend that will present the user with a
 * graphical interface for setting remappings.
 *
 * The remapper currently supports the button layout of an Xbox 360 controller (these are the actual strings used):
 *  Face buttons: A B X Y
 *  Center buttons: Back Guide Start
 *  DPAD: Up Down Left Right
 *  Bumpers: L R
 *  Stick buttons: L3 R3
 *
 * Note that L2 and R2 are not directly remappable, they're analog on the Xbox 360 controller (TODO?)
 */

class Remapper : public Node {
        Q_OBJECT

    public:
        explicit Remapper( Node *parent = nullptr );

    signals:
        // Signals for RemapperModel

        // A new controller GUID was seen, add to the model's list
        void controllerAdded( QString GUID );

        // The last remaining controller with this GUID was removed, do not accept remap requests for this one
        void controllerRemoved( QString GUID );

        // Only fired while not in remap mode, true if any button on any controller with this GUID has been pressed
        void buttonUpdate( QString GUID, bool pressed );

        // Remap mode completed, this GUID now gets this button assigned to it
        void remapModeEnd( QString GUID, QString originalButton, QString remappedButton );

    public slots:
        void commandIn( Command command, QVariant data, qint64 timeStamp ) override;

        void dataIn( DataType type, QMutex *mutex, void *data, size_t bytes, qint64 timeStamp ) override;

        // Do not send input updates to this node's children, listen for a button press on this GUID and reply back
        // with a button once pressed
        void remapModeBegin( QString GUID, QString button );

    private:
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

        // Ensure reads and writes to gamepadBuffer are atomic
        QMutex mutex;

        // A circular buffer that holds gamepad state updates
        // A value of 100 should be sufficient for most purposes
        Gamepad gamepadBuffer[ 100 ];
        int gamepadBufferIndex{ 0 };

        // Remapping stuff

        // Remap data
        QMap<QString, QMap<int, int>> remapData;

        // Count of how many controllers of a certain GUID are connected
        // When 0, the entry may be removed
        // Used to know when it's time to emit controllerConnected()/controllerDisconnected()
        QMap<QString, int> GUIDCount;

        // Helpers

        QString buttonToString( int button );

        int stringToButton( QString button );
};
