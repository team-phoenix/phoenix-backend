#pragma once

#include "controllable.h"
#include "producer.h"

#include "sdleventloop.h"

/*
 * GamepadManager is a QObject that manages input devices. Internally it controls the lifecycle of the SDL library and
 * maintains a list of connected controllers.
 */

class GamepadManager : public QObject, public Producer, public Controllable {
        Q_OBJECT
        Q_PROPERTY( QString controllerDBFile MEMBER controllerDBFile NOTIFY controllerDBFileChanged )

    public:
        explicit GamepadManager( QObject *parent = nullptr );

    public slots:
        void setState( Control::State currentState ) override;

        void poll( qint64 timestamp );

        // Update touch state
        void updateTouchState( QPointF point, bool pressed );

        bool eventFilter( QObject *object, QEvent *event ) override;


    signals:
        PRODUCER_SIGNALS

        void controllerDBFileChanged( QString controllerDBFile );

    private:
        // Current touch state
        QPointF touchCoords;
        bool touchState;

        // A special S-dominated latch, 0 = hold, 1 = set, 2 = reset, 3 = set for 2 frames
        // Specs:
        // If set = reset = 1, output true for this and next frame then false for the 3rd
        // only if for the final 2 frames the input is the normal hold input(set = reset = 0)
        // Sequence begins anew if both inputs are 1 at any time
        int touchLatchState;
        bool touchSet;
        bool touchReset;
        void updateTouchLatch();

        SDLEventLoop m_SDLEventLoop;

        QHash<Qt::Key, Gamepad::Button> m_keyboardMap;

        QVarLengthArray<const Gamepad *, 16> m_gamepadList;
        QVarLengthArray<qint16, 16> m_gamepadStates;
        QVarLengthArray<qint16, 16> m_keyboardStates;

        QString controllerDBFile;

        void installKeyboardFilter();
        void removeKeyboardFilter();

};

inline QHash<Qt::Key, Gamepad::Button > defaultMap() {
    return {
        { Qt::Key_Left, Gamepad::Button::Left },
        { Qt::Key_Right, Gamepad::Button::Right },
        { Qt::Key_Up, Gamepad::Button::Up },
        { Qt::Key_Down, Gamepad::Button::Down },

        { Qt::Key_A, Gamepad::Button::Y },
        { Qt::Key_S, Gamepad::Button::X },
        { Qt::Key_Z, Gamepad::Button::A },
        { Qt::Key_X, Gamepad::Button::B },

        { Qt::Key_Backspace, Gamepad::Button::Select },
        { Qt::Key_Return, Gamepad::Button::Start },

        { Qt::Key_Q, Gamepad::Button::L },
        { Qt::Key_W, Gamepad::Button::R },
        { Qt::Key_E, Gamepad::Button::L2 },
        { Qt::Key_R, Gamepad::Button::R2 },
        { Qt::Key_Control, Gamepad::Button::R3 },
        { Qt::Key_Shift, Gamepad::Button::L3 },
      };
}

