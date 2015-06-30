#ifndef INPUTDEVICE
#define INPUTDEVICE

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QDebug>
#include <QMap>
#include <QQmlEngine>
#include <QVariantMap>
#include <QSettings>
#include <QFile>
#include <memory>

#include "libretro.h"
#include "logging.h"
#include "inputdeviceevent.h"

// InputDevice represents an abstract controller.

// In normal situations, you only need to
// reimplement the insert function.

// If an InputDevice was dynamically allocated, which it should be, don't call 'delete inputDevice', instead do
// 'inputDevice->selfDestruct'. This is because changes to the InputDevice's mapping are only written to a save file
// when the application closes.

// Use alias so we don't have to type this out every time. :/
using InputStateMap = QHash<InputDeviceEvent::Event, int16_t>;

class InputDevice : public QObject {
        Q_OBJECT
        Q_PROPERTY( QString name READ name WRITE setName NOTIFY nameChanged )
        Q_PROPERTY( int retroButtonCount READ retroButtonCount NOTIFY retroButtonCountChanged )
        Q_PROPERTY( bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged )
        Q_PROPERTY( bool resetMapping READ resetMapping WRITE setResetMapping NOTIFY resetMappingChanged )

    public:

        // This should be turned off when a game is running
        // and set to true when the game stops. The setRun function of InputManager toggles this.
        static bool gamepadControlsFrontend;

        // Controller types from libretro's perspective
        enum  LibretroType {
            DigitalGamepad = RETRO_DEVICE_JOYPAD,
            AnalogGamepad = RETRO_DEVICE_ANALOG,
        };

        // For a normal InputDevice subclass, don't just call this constructor. This should
        // only be used by the QMLInputDevice.
        explicit InputDevice( QObject *parent = 0 );

        explicit InputDevice( const LibretroType type, QObject *parent = 0 );

        explicit InputDevice( const LibretroType type, const QString name, QObject *parent );

        // Getters

        // editMode stops incoming button events from being put into the deviceStates mapping. It causes the insert()
        // function to emit inputDeviceEventChanged(), which allows the frontend to connect and react to.
        bool editMode() const; // QML
        const QString name() const; // QML
        int retroButtonCount() const; // QML
        QString mappingString() const;
        bool resetMapping() const;// QML
        LibretroType type() const;
        InputStateMap *states();

        // Setters
        void setName( const QString name ); // QML
        void setEditMode( const bool edit ); // QML
        void setResetMapping( const bool reset ); // QML

        void setType( const LibretroType type );

        virtual void saveMapping();

        virtual bool loadMapping();

        // This is the only you we should be deleting the InputDevice.
        // This function calls saveMapping() before it deletes itself.
        void selfDestruct();

    public slots:

        // Poll button state (getter)
        virtual int16_t value( const InputDeviceEvent::Event &event, const int16_t defaultValue = 0 );

        // Set button state (setter)
        virtual void insert( const InputDeviceEvent::Event &value, const int16_t &state );

        // Set the device -> SDL2 gamepad mapping
        virtual void setMapping( const QVariantMap mapping );

    protected:

        // The device's current state (whether certain buttons are pressed)
        std::unique_ptr<InputStateMap> deviceStates;

    signals:

        void editModeChanged(); // QML
        void nameChanged(); // QML
        void retroButtonCountChanged(); // QML
        void resetMappingChanged(); // QML

        // The inputDeviceEvent signal is used to connect to the QMLInputDevice
        // and shouldn't be connected to anything else.
        void inputDeviceEvent( InputDeviceEvent::Event event, int state ); // QML

        // The editModeEvent signal is used for changing the InputDevice's internal button map.
        // This should be connected to any time the user wants to change the mapping.
        // After this mapping has been edited, this signal can be disconnected.
        void editModeEvent( int event, int state ); // QML

    private:

        // Type of controller this input device is
        LibretroType deviceType;

        // Controller states are read by a different thread, lock access with a mutex
        QMutex mutex;

        // Clear button states
        void resetStates();
        void setRetroButtonCount( const int count );

        // QML Variables
        QString deviceName;
        QString qmlMappingString;
        int qmlRetroButtonCount;
        bool qmlEditMode;
        bool qmlResetMapping;

};

Q_DECLARE_METATYPE( InputDevice * )

#endif // INPUTDEVICE

