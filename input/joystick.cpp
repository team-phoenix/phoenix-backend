#include "joystick.h"

const int Joystick::maxNumOfDevices = 8;

Joystick::Joystick( const int joystickIndex, QObject *parent )
    : InputDevice( LibretroType::DigitalGamepad, parent ),
      qmlDeadZone( 12000 ),
      qmlAnalogMode( false ),
      mSDLButtonVector( SDL_CONTROLLER_BUTTON_MAX, SDL_CONTROLLER_BUTTON_INVALID ),
      mSDLAxisVector( SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_INVALID ) {

    setPort( joystickIndex );
    device = SDL_GameControllerOpen( joystickIndex );
    setName( SDL_GameControllerName( device ) );
    qmlInstanceID = SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( device ) );

    auto *sdlJoystick = SDL_GameControllerGetJoystick( device );
    qmlAxisCount = SDL_JoystickNumAxes( sdlJoystick );
    qmlButtonCount = SDL_JoystickNumButtons( sdlJoystick );

    qmlHatCount = SDL_JoystickNumHats( sdlJoystick );
    qmlBallCount = SDL_JoystickNumBalls( sdlJoystick );

    char guidStr[1024];
    SDL_JoystickGUID guid = SDL_JoystickGetGUID( sdlJoystick );
    SDL_JoystickGetGUIDString( guid, guidStr, sizeof( guidStr ) );

    qmlGuid = guidStr;

    mDigitalTriggers = hasDigitalTriggers( qmlGuid );

    // This is really annoying, but for whatever reason, the SDL2 Game Controller API,
    // doesn't assign a proper mapping value certain controller buttons.
    // This means that we have to hold the mapping ourselves and do it correctly.

    connect( this, &Joystick::resetMappingChanged, this, [ this ] {
        if( resetMapping() )
            loadSDLMapping( sdlDevice() );
    } );

    if( !loadSDLMapping( nullptr ) ) {
        Q_ASSERT( loadSDLMapping( sdlDevice() ) );
    }

}

Joystick::~Joystick() {
    close();
}

QString Joystick::guid() const {
    return qmlGuid;
}

int Joystick::buttonCount() const {
    return qmlButtonCount;
}

int Joystick::ballCount() const {
    return qmlBallCount;
}

int Joystick::hatCount() const {
    return qmlHatCount;
}

int Joystick::axisCount() const {
    return qmlAxisCount;
}

qreal Joystick::deadZone() const {
    return qmlDeadZone;
}

bool Joystick::analogMode() const {
    return qmlAnalogMode;
}

bool Joystick::digitalTriggers() const {
    return mDigitalTriggers;
}

quint8 Joystick::getButtonState( const SDL_GameControllerButton &button ) {

    if( button >= mSDLButtonVector.size() ) {
        return 0;
    }

    auto buttonID = mSDLButtonVector.at( button );

    return SDL_JoystickGetButton( sdlJoystick(), buttonID );

}

qint16 Joystick::getAxisState( const SDL_GameControllerAxis &axis ) {

    if( axis >= mSDLAxisVector.size() ) {
        return 0;
    }

    auto axisID = mSDLAxisVector.at( axis );

    switch( axis ) {

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            if( digitalTriggers() ) {
                return SDL_JoystickGetButton( sdlJoystick(), axisID );
            }

            return SDL_JoystickGetAxis( sdlJoystick(),  axisID );

        default:
            return SDL_JoystickGetAxis( sdlJoystick(),  axisID );

    }

}

void Joystick::setAnalogMode( const bool mode ) {
    qmlAnalogMode = mode;
}

SDL_GameController *Joystick::sdlDevice() const {
    return device;
}

SDL_Joystick *Joystick::sdlJoystick() const {
    return SDL_GameControllerGetJoystick( device );
}

SDL_JoystickID Joystick::instanceID() const {
    return qmlInstanceID;
}

void Joystick::close() {
    Q_ASSERT_X( device, "InputDevice" , "the device was deleted by an external source" );
    SDL_GameControllerClose( device );
}

bool Joystick::loadMapping() {

    return false;


}

void Joystick::emitEditModeEvent( int event, int state, const InputDeviceEvent::EditEventType type ) {
    emit editModeEvent( event, state, type );
}

void Joystick::emitInputDeviceEvent( InputDeviceEvent::Event event, int state ) {
    emit inputDeviceEvent( event, state );
}

bool Joystick::hasDigitalTriggers( const QString &guid ) {

    if( guid == QStringLiteral( "050000005769696d6f74652028313800" ) ) {
        return true;
    }

    return false;

}

void Joystick::fillSDLArrays( const QString &key, const int &numberValue ) {
    auto byteArray = key.toLocal8Bit();

    if( key == QStringLiteral( "leftx" )
        || key == QStringLiteral( "lefty" )
        || key == QStringLiteral( "rightx" )
        || key == QStringLiteral( "righty" )
        || key == QStringLiteral( "lefttrigger" )
        || key == QStringLiteral( "righttrigger" ) ) {
        mSDLAxisVector[ SDL_GameControllerGetAxisFromString( byteArray.constData() ) ] = numberValue;
    }

    else {

        mSDLButtonVector[ SDL_GameControllerGetButtonFromString( byteArray.constData() ) ] = numberValue;

    }
}

void Joystick::setMappings( const QVariant key, const QVariant newMapping, const InputDeviceEvent::EditEventType type ) {
    mappingRef().insert( key.toString(), newMapping );

    auto byteArray = key.toString().toLocal8Bit();

    if( type == InputDeviceEvent::EditEventType::ButtonEvent ) {
        mSDLButtonVector[ SDL_GameControllerGetButtonFromString( byteArray.constData() ) ] = newMapping.toInt();
    } else if( type == InputDeviceEvent::EditEventType::AxisEvent ) {
        mSDLAxisVector[ SDL_GameControllerGetAxisFromString( byteArray.constData() ) ] = newMapping.toInt();
    }

}

void Joystick::saveMappings() {

    if( mappingRef().isEmpty() ) {
        return;
    }

    QString mappingString = guid() + QStringLiteral( "," ) + name() + QStringLiteral( "," );

    for( auto i = 0; i < mSDLButtonVector.size(); ++i ) {
        const auto eventString = SDL_GameControllerGetStringForButton( static_cast<SDL_GameControllerButton>( i ) );
        mappingString += eventString + QStringLiteral( ":" ) + QStringLiteral( "b" )
                         + QString::number( mSDLButtonVector.value( i ) ) + QStringLiteral( "," );
    }

    for( auto i = 0; i < mSDLAxisVector.size(); ++i ) {
        const auto eventString = SDL_GameControllerGetStringForAxis( static_cast<SDL_GameControllerAxis>( i ) );
        mappingString += eventString + QStringLiteral( ":" ) + QStringLiteral( "a" )
                         + QString::number( mSDLAxisVector.value( i ) ) + QStringLiteral( "," );
    }

#if defined( Q_OS_WIN32 )
    const QString platform = QStringLiteral( "Windows" );
#elif defined( Q_OS_MACX )
    const QString platform = QStringLiteral( "Mac OS X" );
#elif defined( Q_OS_LINUX )
    const QString platform = QStringLiteral( "Linux" );
#endif

    mappingString += QStringLiteral( "platform:" ) + platform;

    QSettings settings;
    settings.beginGroup( QStringLiteral( "Input" ) );
    settings.setValue( guid(), mappingString );

}

bool Joystick::loadSDLMapping( SDL_GameController *device ) {

    // Handle populating our own mappings, because SDL2 often uses the incorrect mapping array.

    QString mappingString;

    if( device ) {
        mappingString = SDL_GameControllerMapping( device );
    } else {
        QSettings settings;
        settings.beginGroup( QStringLiteral( "Input" ) );
        auto variant = settings.value( guid() );

        if( variant.isValid() ) {
            mappingString = variant.toString();
        } else {
            qCWarning( phxLibrary ) << "Settings for " << name()
                                    << "are invalid. Not settings were mapped.";
            return false;
        }
    }

    auto strList = mappingString.split( "," );

    for( QString &str : strList ) {

        auto keyValuePair = str.split( ":" );

        if( keyValuePair.size() <= 1 ) {
            continue;
        }

        auto key = keyValuePair.at( 0 );
        auto value = keyValuePair.at( 1 );

        if( value.isEmpty() ) {
            qCWarning( phxInput ) << "The value for " << key << " is empty.";
            continue;
        }

        if( key == QStringLiteral( "platform" ) ) {
            continue;
        }

        auto prefix = value.at( 0 );
        int numberValue = value.remove( prefix ).toInt();

        mappingRef().insert( key, numberValue );

        fillSDLArrays( key, numberValue );

    }

    return !mapping().isEmpty();
}

