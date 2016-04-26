#include "gamepad.h"

#include <QDebug>
#include <QVariant>

Gamepad::Gamepad(int index)
    : m_buttonStates( static_cast<int>( Gamepad::Button::Max ) ),
      m_axisStates( static_cast<int>( Gamepad::Axis::Max ) ),
      m_isKeyboard( index < 0 )
{
    if ( m_isKeyboard ) {
        m_name = QStringLiteral( "Keyboard" );
        m_id = index;
        return;
    }

    m_SDLGamepad =  SDL_GameControllerOpen( index );
    m_SDLJoystick = SDL_GameControllerGetJoystick( m_SDLGamepad );
    m_name = SDL_GameControllerName( m_SDLGamepad );
    m_id = SDL_JoystickInstanceID( m_SDLJoystick );

    for ( int i=0; i < static_cast<int>( Gamepad::Button::Max ); ++i ) {
        m_buttonStates[ i ] = 0;
    }

    for ( int i=0; i < static_cast<int>( Gamepad::Axis::Max ); ++i ) {
        m_axisStates[ i ] = 0;
    }

}

Gamepad::~Gamepad() {
    if ( m_isKeyboard ) {
        SDL_GameControllerClose( m_SDLGamepad );
    }
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

QMap<QString, QString> Gamepad::mapping() const {
    QMap<QString, QString> _result;

    if ( !m_isKeyboard ) {
        for ( int button=0; button < SDL_CONTROLLER_BUTTON_MAX; ++button ) {
            auto _gPadButton = static_cast<SDL_GameControllerButton>( button );
            QString val = SDL_GameControllerGetStringForButton( static_cast<SDL_GameControllerButton>( SDL_GameControllerGetBindForButton(
                                                                    m_SDLGamepad
                                                                    , _gPadButton ).value.button ) );
            _result[ toString( toGamepadButton( button ) ) ] = val;
        }

        qDebug() << _result;
        return _result;
    }


    return _result;
}

QString Gamepad::name() const {
    return m_name;
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

void Gamepad::setMapping(QVariantMap t_mapping) {
   // for ( auto iter = t_mapping.begin(); iter < t_mapping.end(); ++iter ) {
   //     qDebug() << Q_FUNC_INFO << iter.key() << iter.value().toString();
    //}
}

void Gamepad::updateButtonState(Gamepad::Button t_button, qint16 state) {
    auto _button = static_cast<int>( t_button );
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

QString toString(Gamepad::Button button) {
    return [&button] {
        switch( button ) {
        case Gamepad::Button::Invalid:
            return QStringLiteral( "invalid" );
        case Gamepad::Button::B:
            return QStringLiteral( "b" );
        case Gamepad::Button::A:
            return QStringLiteral( "a" );
        case Gamepad::Button::Y:
            return QStringLiteral( "y" );
        case Gamepad::Button::X:
            return QStringLiteral( "x" );
        case Gamepad::Button::Select:
            return QStringLiteral( "select" );
        case Gamepad::Button::Guide:
            return QStringLiteral( "guide" );
        case Gamepad::Button::Start:
            return QStringLiteral( "start" );
        case Gamepad::Button::L3:
            return QStringLiteral( "l3" );
        case Gamepad::Button::R3:
            return QStringLiteral( "r3" );
        case Gamepad::Button::L:
            return QStringLiteral( "l" );
        case Gamepad::Button::R:
            return QStringLiteral( "r" );
        case Gamepad::Button::R2:
            return QStringLiteral( "r2" );
        case Gamepad::Button::L2:
            return QStringLiteral( "l2" );
        case Gamepad::Button::Up:
            return QStringLiteral( "up" );
        case Gamepad::Button::Down:
            return QStringLiteral( "down" );
        case Gamepad::Button::Left:
            return QStringLiteral( "left" );
        case Gamepad::Button::Right:
            return QStringLiteral( "right" );
        case Gamepad::Button::Max:
            return QStringLiteral( "max" );
        default:
            Q_UNREACHABLE();
        }
    }();
}
