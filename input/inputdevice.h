#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

/*

#include "producer.h"
#include "logging.h"

#include "iter.h"

// InputDevice represents an abstract controller.

// In normal situations, you only need to
// reimplement the insert function.

// If an InputDevice was dynamically allocated, which it should be, don't call 'delete inputDevice', instead do
// 'inputDevice->selfDestruct'. This is because changes to the InputDevice's mapping are only written to a save file
// when the application closes.

// Use alias so we don't have to type this out every time. :/

using namespace Input;

using DigitalButtons = Libretro::Gamepad::DigitalButtons;
using AnalogButtons = Libretro::Gamepad::AnalogButtons;
using GamepadType = Libretro::Gamepad::Type;

class InputDevice : public QObject {
    Q_OBJECT
    public:
        using InputMap = QHash<int, InputMapValue>;
        using StateList = QVector<int>;
        using AnalogStateList = StateList;
        using DigitalStateList = StateList;

        InputDevice() = default;
        InputDevice( int port, GamepadType type );

        virtual ~InputDevice() = default;

        void remapInput( int key, const InputMapValue &value );
        void remapInput( int key, InputMapValue &&value );
        void remapInput( const InputMap &map );
        void remapInput( InputMap &&map );

        int getDigitalState( int type );
        int getAnalogState( int type );

        const QString name() const; // QML
        int port() const;
        GamepadType type() const;

        // Maybe delete this
        QString mappingString() const;

        void setName( const QString name ); // QML

    public slots:

        // Poll button state (getter)

        // Set button state (setter)
        //void insert( const InputDeviceEvent &event );
        //void insert( InputDeviceEvent &&event );

       // virtual void insert( const InputDeviceEvent::Event &value, const int16_t &state );

        // Set the device -> SDL2 gamepad mapping
        //virtual bool setMappings( const QVariant key, const QVariant mapping, const InputDeviceEvent::EditEventType ) = 0;

        bool mappingCollision( const QString key, const QVariant mapping ) {
            Q_UNUSED( key );
            Q_UNUSED( mapping );
            return false;
        }

    protected:
        DigitalStateList mDigitalStates;
        AnalogStateList mAnalogStates;

        InputMap mInputMap;

        QString mName{ QStringLiteral( "name_undefined_see_inputdevice.h" ) };
        int mPort{ 0 };
        GamepadType mType{ GamepadType::Digital };

    signals:
        // See producer.h
        //PRODUCER_SIGNALS

    private:

        // Type of controller this input device is
        GamepadType deviceType{ GamepadType::Digital };

        QString qmlMappingString;
        bool qmlEditMode{ false };
        bool qmlResetMapping{ false };


        inline void reset( StateList &vec ) {
            for( int i{0}; i < vec.size(); ++i ) {
                vec[ i ] = 0;
            }
        }

};

Q_DECLARE_METATYPE( InputDevice * )
*/

#endif // INPUTDEVICE_H

