#include "gamepad.h"
#include "logging.h"

#include "libretro.h"

Gamepad::Gamepad() :
    m_sdlController( nullptr ),
    m_sdlJoystick( nullptr ),
    m_buttonStates( SDL_CONTROLLER_BUTTON_MAX, 0 ),
    m_axisStates( SDL_CONTROLLER_AXIS_MAX, 0 ),
    m_buttonMapping( SDL_CONTROLLER_BUTTON_MAX + 1, SDL_CONTROLLER_BUTTON_INVALID )
{

}

void Gamepad::open(int t_joystickIndex) {

    m_sdlController = SDL_GameControllerOpen( t_joystickIndex );
    if ( !m_sdlController ) {
        const QString message = QString( "Could not open SDL_Gamepad %0: %1" ).arg(
                    QString::number( t_joystickIndex )
                    , QString( SDL_GetError() ) );

        throw std::runtime_error( message.toStdString() );
    }

    m_sdlJoystick = SDL_GameControllerGetJoystick( m_sdlController );

    m_name = SDL_GameControllerName( m_sdlController );

    for ( int i=RETRO_DEVICE_ID_JOYPAD_B; i < RETRO_DEVICE_ID_JOYPAD_R3 + 1; ++i ) {
        switch( i ) {
            case RETRO_DEVICE_ID_JOYPAD_B:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_B;
                break;
            case RETRO_DEVICE_ID_JOYPAD_Y:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_Y;
                break;
            case RETRO_DEVICE_ID_JOYPAD_SELECT:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_BACK;

                break;
            case RETRO_DEVICE_ID_JOYPAD_START:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_START;

                break;
            case RETRO_DEVICE_ID_JOYPAD_UP:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_DPAD_UP;
                break;
            case RETRO_DEVICE_ID_JOYPAD_DOWN:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_DPAD_DOWN;

                break;
            case RETRO_DEVICE_ID_JOYPAD_LEFT:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_DPAD_LEFT;

                break;
            case RETRO_DEVICE_ID_JOYPAD_RIGHT:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

                break;
            case RETRO_DEVICE_ID_JOYPAD_A:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_A;
                break;
            case RETRO_DEVICE_ID_JOYPAD_X:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_X;

                break;
            case RETRO_DEVICE_ID_JOYPAD_L:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;

                break;
            case RETRO_DEVICE_ID_JOYPAD_R:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

                break;
            case RETRO_DEVICE_ID_JOYPAD_L2:
                break;
            case RETRO_DEVICE_ID_JOYPAD_R2:
                break;
            case RETRO_DEVICE_ID_JOYPAD_L3:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_LEFTSTICK;
                break;
            case RETRO_DEVICE_ID_JOYPAD_R3:
                m_buttonMapping[ i ] = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
                break;
            default:
                Q_UNREACHABLE();
        }
    }

}

void Gamepad::updateStates() {
    for ( int i=0; i < SDL_CONTROLLER_BUTTON_MAX; ++i ) {
        quint8 state = SDL_GameControllerGetButton( m_sdlController, static_cast<SDL_GameControllerButton>( i ) );
        m_buttonStates[ i ] = state;
    }

    for ( int i=0; i < SDL_CONTROLLER_AXIS_MAX; ++i ) {
        qint16 state = SDL_GameControllerGetAxis( m_sdlController, static_cast<SDL_GameControllerAxis>( i ) );
        m_axisStates[ i ] = state;
    }
}

quint8 Gamepad::getButtonState(SDL_GameControllerButton t_button) const {
    return m_buttonStates[ t_button ];
}

qint16 Gamepad::getAxisState(SDL_GameControllerAxis t_axis) {
    return m_axisStates[ t_axis ];
}

const QString &Gamepad::name() const {
    return m_name;
}

int Gamepad::instanceID() const {
    return SDL_JoystickInstanceID( m_sdlJoystick );
}

bool Gamepad::isAttached() const {
    return SDL_GameControllerGetAttached( m_sdlController );
}

Gamepad::~Gamepad() {
    if ( m_sdlController ) {
        SDL_GameControllerClose( m_sdlController );
    }
}

quint8 Gamepad::getButtonState(uint t_libretroID ) const {
    SDL_GameControllerButton button =  m_buttonMapping[ t_libretroID ];

    return ( button == SDL_CONTROLLER_BUTTON_INVALID ) ? 0 : getButtonState( button );
}
