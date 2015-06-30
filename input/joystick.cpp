#include "joystick.h"

const int Joystick::maxNumOfDevices = 128;

Joystick::Joystick( const int joystickIndex, QObject *parent )
    : InputDevice( LibretroType::DigitalGamepad, parent ),
      qmlSdlIndex( joystickIndex ),
      qmlDeadZone( 12000 ),
      qmlAnalogMode( false ),
      mSDLButtonVector( SDL_CONTROLLER_BUTTON_MAX, SDL_CONTROLLER_BUTTON_INVALID ),
      mSDLAxisVector( SDL_CONTROLLER_BUTTON_MAX, SDL_CONTROLLER_BUTTON_INVALID ) {

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

    loadSDLMapping( sdlDevice() );

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

int Joystick::sdlIndex() const {
    return qmlSdlIndex;
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

QHash<QString, int> &Joystick::sdlMapping() {
    return sdlControllerMapping;
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

void Joystick::setSDLIndex( const int index ) {
    qmlSdlIndex = index;
}

void Joystick::close() {
    Q_ASSERT_X( device, "InputDevice" , "the device was deleted by an external source" );
    SDL_GameControllerClose( device );
}

bool Joystick::loadMapping() {

    return false;

    /*
        QSettings settings;
        settings.beginGroup( guid() );

        // No point in iterating if the file doesn't exist.
        if ( !QFile::exists( settings.fileName() ) )
            return false;

        // Check for button saves.
        for ( int i=0; i < SDL_CONTROLLER_BUTTON_MAX; ++i ) {
            auto button = static_cast<SDL_GameControllerButton>( i );

            auto buttonString = QString( SDL_GameControllerGetStringForButton( button ) );

            auto value = settings.value( buttonString );

            if ( value.isValid() ) {
                int numberValue = value.toInt();

                auto event = sdlStringToEvent( buttonString );

                if ( event != InputDeviceEvent::Unknown) {
                    mapping().insert( buttonString, event );
                    sdlControllerMapping.insert( buttonString, numberValue );
                    //qDebug() << buttonString << numberValue;
                }
            }
        }

        // Check for axis saves.
        for ( int i=0; i < SDL_CONTROLLER_AXIS_MAX; ++i ) {
            auto axis = static_cast<SDL_GameControllerAxis>( i );

            auto axisString = QString( SDL_GameControllerGetStringForAxis( axis ) );

            auto value = settings.value( axisString );
            if ( value.isValid() ) {
                int numberValue = value.toInt();

                auto event = sdlStringToEvent( axisString );
                if ( event != InputDeviceEvent::Unknown) {
                    mapping().insert( axisString, event );
                    sdlControllerMapping.insert( axisString, numberValue );
                }
            }
        }

        return !mapping().isEmpty();
        */

}

void Joystick::saveMapping() {

}

void Joystick::emitEditModeEvent( int event, int state ) {
    emit editModeEvent( event, state );
}

void Joystick::emitInputDeviceEvent( InputDeviceEvent::Event event, int state ) {
    emit inputDeviceEvent( event, state );
}

bool Joystick::hasDigitalTriggers( const QString &guid ) {

    if( guid == "050000005769696d6f74652028313800" ) {
        return true;
    }

    return false;

}

void Joystick::setMapping( const QVariantMap newMapping ) {
    Q_UNUSED( newMapping );

    /*
    for ( auto &newEvent : newMapping.keys() ) {
        auto newValue = newMapping.value( newEvent ).toInt();

        bool foundCollision = false;
        for ( auto &oldEvent : sdlControllerMapping.keys() ) {
            auto oldValue = sdlControllerMapping.value( oldEvent );

            // Override old value.
            if ( newEvent == oldEvent) {

                foundCollision = true;

                // button collisions will have to be noticed by the user for the time being.
                // This is because the InputDeviceEvent is <QString, int> instead
                // of <QString, QString>.
                qDebug() << name() << newEvent << ":" << oldValue  << "==>" << newValue;
                //sdlControllerMapping[ newEvent ] = newValue;
                break;
            }
        }

        if ( !foundCollision ) {
            //sdlControllerMapping.insert( newEvent, newValue );
        }

    }
    */

}

void Joystick::loadSDLMapping( SDL_GameController *device ) {

    // Handle populating our own mappings, because SDL2 often uses the incorrect mapping array.

    QString mappingString = SDL_GameControllerMapping( device );

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

        if( key == "platform" ) {
            continue;
        }

        auto prefix = value.at( 0 );
        int numberValue = value.remove( prefix ).toInt();
        auto byteArray = key.toLocal8Bit();

        if( key == "leftx"
            || key == "lefty"
            || key == "rightx"
            || key == "righty"
            || key == "lefttrigger"
            || key == "righttrigger" ) {
            mSDLAxisVector[ SDL_GameControllerGetAxisFromString( byteArray.constData() ) ] = numberValue;
        }

        else {

            if( prefix == 'a' ) {

                qCWarning( phxInput ) << key
                                      << " has an unhandled axis value. Report this to the Phoenix "
                                      << " developers.";
                continue;

            }

            mSDLButtonVector[ SDL_GameControllerGetButtonFromString( byteArray.constData() ) ] = numberValue;

        }

    }

}
