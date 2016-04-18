#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QKeyEvent>

// This class represents one Qt keyboard.
// This class connects to the the window's keyPressEvent()
// and keyReleaseEvent() functions, to handle incoming key values
// and turn the into valid RETRO_PAD button.


namespace Input {

    class Keyboard : public QObject {
        Q_OBJECT
        public:

        ~Keyboard() = default;

        public slots:

            //bool setMappings( const QVariant key, const QVariant mapping, const InputDeviceEvent::EditEventType type ) override;
            //void saveMappings() override;

        private:

            void loadDefaultMapping();

            /*
            inline InputMap defaultMap() {
                return {
                    { Qt::Key_Left, { DigitalButtons::Left, "Left" } },
                    { Qt::Key_Right, { DigitalButtons::Right, "Right" } },
                    { Qt::Key_Up, { DigitalButtons::Up, "Up" } },
                    { Qt::Key_Down, { DigitalButtons::Down, "Down" } },

                    { Qt::Key_A, { DigitalButtons::Y, "Y" } },
                    { Qt::Key_S, { DigitalButtons::X, "X" } },
                    { Qt::Key_Z, { DigitalButtons::A, "A" } },
                    { Qt::Key_X, { DigitalButtons::B, "B" } },

                    { Qt::Key_Backspace, { DigitalButtons::Select, "Select" } },
                    { Qt::Key_Return, { DigitalButtons::Start, "Start" } },

                    { Qt::Key_Q, { DigitalButtons::L, "L" } },
                    { Qt::Key_W, { DigitalButtons::R, "R" } },
                    { Qt::Key_E, { DigitalButtons::L2, "L2" } },
                    { Qt::Key_R, { DigitalButtons::R2, "R2" } },
                    { Qt::Key_Control, { DigitalButtons::R3, "R3" } },
                    { Qt::Key_Shift, { DigitalButtons::L3, "L3" } },
                  };
            }
            */

    };

}

#endif // KEYBOARD_H
