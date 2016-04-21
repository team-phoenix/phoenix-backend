#pragma once

#include "pipelinenode.h"
#include "controllable.h"
#include "producer.h"
#include "sdleventloop.h"

/*
 * GamepadManager is a QObject that manages input devices. Internally it controls the lifecycle of the SDL library and
 * maintains a list of connected controllers.
 */

class GamepadManager : public QObject, public Producer, public Controllable {
        Q_OBJECT
        PHX_PIPELINE_INTERFACE( GamepadManager )
        Q_PROPERTY( QString controllerDBFile MEMBER controllerDBFile NOTIFY controllerDBFileChanged )

    public:
        explicit GamepadManager( QObject *parent = nullptr );
        ~GamepadManager();

    public slots:
        void setState( Control::State currentState ) override;

        void poll( QMutex *t_mutex
                   , void *t_data
                   , size_t &t_bytes
                   , qint64 &t_timeStamp );

        // Update touch state
        void updateTouchState( QPointF point, bool pressed );

        bool eventFilter( QObject *object, QEvent *event ) override;

        void addGamepad( const Gamepad *_gamepad );

        void dataIn( PipelineNode::DataReason t_reason
                     , QMutex *t_mutex
                     , void *t_data
                     , size_t t_bytes
                     , qint64 t_timeStamp ) {

            switch ( t_reason ) {
            case PipelineNode::Poll_Input_nullptr:
                m_SDLEventLoop.poll();
                break;
            case PipelineNode::Create_Frame_any:
                poll( t_mutex, t_data, t_bytes, t_timeStamp );
            default:
                break;
            }

            emitDataOut( t_reason, t_data, t_bytes, t_timeStamp );
        }

    signals:
        PRODUCER_SIGNALS
        void gamepadAdded( const Gamepad * );
        void gamepadRemoved( const Gamepad * );

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
        qint16 m_gamepadStates[ 16 ][ 16];
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
