#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "inputdevice.h"
#include "inputdeviceevent.h"

// This class represents one Qt keyboard.
// This class connects to the the window's keyPressEvent()
// and keyReleaseEvent() functions, to handle incoming key values
// and turn the into valid RETRO_PAD button.

using InputDeviceMapping = QHash< int, InputDeviceEvent::Event >;

class Keyboard : public InputDevice {
        Q_OBJECT

    public:

        // This needs to be included for proper lookup during compliation.
        // Else the compiler will throw a warning.
        using InputDevice::insert;

        explicit Keyboard( QObject *parent = 0 );

        InputDeviceMapping &mapping();

        bool loadMapping() override;
        void saveMapping() override;

    public slots:

        void insert( const int &event, int16_t pressed );
        void setMapping( const QVariantMap mapping ) override;

    private:

        void loadDefaultMapping();

        InputDeviceMapping deviceMapping;

};

#endif // KEYBOARD_H
