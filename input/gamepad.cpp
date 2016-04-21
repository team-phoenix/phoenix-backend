#include "gamepad.h"

#include <QDebug>

Gamepad::Gamepad(int index)
    : m_buttonStates( static_cast<int>( Gamepad::Button::Max ) ),
      m_axisStates( static_cast<int>( Gamepad::Axis::Max ) )
{
    m_SDLGamepad =  SDL_GameControllerOpen( index );
    m_SDLJoystick = SDL_GameControllerGetJoystick( m_SDLGamepad );
    m_id = SDL_JoystickInstanceID( m_SDLJoystick );

    for ( int i=0; i < static_cast<int>( Gamepad::Button::Max ); ++i ) {
        m_buttonStates[ i ] = 0;
    }

    for ( int i=0; i < static_cast<int>( Gamepad::Axis::Max ); ++i ) {
        m_axisStates[ i ] = 0;
    }

}

Gamepad::~Gamepad() {
    SDL_GameControllerClose( m_SDLGamepad );
}

void Gamepad::update() {

    // Update button states
    for ( int button=0; button < SDL_CONTROLLER_BUTTON_MAX; ++button ) {
        auto state = SDL_GameControllerGetButton( m_SDLGamepad
                                                  , static_cast<SDL_GameControllerButton>( button ) );
        updateButtonState( toGamepadButton( button ), state );

    }

    // Update axis states
    for( int axis=0; axis < SDL_CONTROLLER_AXIS_MAX; ++axis ) {
        auto state = SDL_JoystickGetAxis( m_SDLJoystick, static_cast<SDL_GameControllerAxis>( axis ) );
        auto gAxis = toGamepadAxis( axis );
        if ( state != axisState( gAxis ) ) {
            updateAxisState( gAxis, state );
        }
    }

}

QString Gamepad::mapping() const {
    return SDL_GameControllerMapping( m_SDLGamepad );
}

qint32 Gamepad::id() const
{
    return m_id;
}

bool Gamepad::isOpen() const
{
    return SDL_GameControllerOpen( id() );
}

qint16 Gamepad::buttonState(Gamepad::Button t_button) const {
    return m_buttonStates[ static_cast<int>( t_button ) ];
}

qint16 Gamepad::axisState(Gamepad::Axis t_axis) const {
    return m_axisStates[ static_cast<int>( t_axis ) ];
}

void Gamepad::updateButtonState(Gamepad::Button t_button, qint16 state) {
    auto _button = static_cast<int>( t_button );
    if ( _button != m_buttonStates[ _button ] ) {
        switch( t_button ) {
        case Gamepad::Button::A:
            setA( state );
            break;
        case Gamepad::Button::B:
            setB( state );
            break;
        case Gamepad::Button::X:
            setX( state );
            break;
        case Gamepad::Button::Y:
            setY( state );
            break;
        default:
            //Q_UNREACHABLE();
            break;
        }
    }
    m_buttonStates[ _button ] = state;

}

void Gamepad::updateAxisState(Gamepad::Axis axis, qint16 state) {
    m_axisStates[ static_cast<int>( axis ) ] = state;
}

Gamepad::Axis toGamepadAxis(int axis) {
    return [&axis] {
        switch( static_cast<SDL_GameControllerAxis>( axis ) ) {
        case SDL_CONTROLLER_AXIS_INVALID:
            return Gamepad::Axis::Invalid;
        case SDL_CONTROLLER_AXIS_LEFTX:
            return Gamepad::Axis::LeftX;
        case SDL_CONTROLLER_AXIS_LEFTY:
            return Gamepad::Axis::LeftY;
        case SDL_CONTROLLER_AXIS_RIGHTX:
            return Gamepad::Axis::RightX;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            return Gamepad::Axis::RightY;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            return Gamepad::Axis::LeftTrigger;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            return Gamepad::Axis::RightTrigger;
        case SDL_CONTROLLER_AXIS_MAX:
            return Gamepad::Axis::Max;
        default:
            Q_UNREACHABLE();
        }
    }();
}

Gamepad::Button toGamepadButton( int button ) {
    return [&button] {
        switch( static_cast<SDL_GameControllerButton>( button ) ) {
        case SDL_CONTROLLER_BUTTON_INVALID:
            return Gamepad::Button::Invalid;
        case SDL_CONTROLLER_BUTTON_A:
            return Gamepad::Button::B;
        case SDL_CONTROLLER_BUTTON_B:
            return Gamepad::Button::A;
        case SDL_CONTROLLER_BUTTON_X:
            return Gamepad::Button::Y;
        case SDL_CONTROLLER_BUTTON_Y:
            return Gamepad::Button::X;
        case SDL_CONTROLLER_BUTTON_BACK:
            return Gamepad::Button::Select;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return Gamepad::Button::Guide;
        case SDL_CONTROLLER_BUTTON_START:
            return Gamepad::Button::Start;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return Gamepad::Button::L3;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return Gamepad::Button::R3;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return Gamepad::Button::L;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return Gamepad::Button::R;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return Gamepad::Button::Up;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return Gamepad::Button::Down;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return Gamepad::Button::Left;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return Gamepad::Button::Right;
        case SDL_CONTROLLER_BUTTON_MAX:
            return Gamepad::Button::Max;
        default:
            Q_UNREACHABLE();
        }
    }();
}
