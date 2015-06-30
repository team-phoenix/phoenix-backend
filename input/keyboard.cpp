#include "keyboard.h"

Keyboard::Keyboard( QObject *parent )
    : InputDevice( LibretroType::DigitalGamepad, "Keyboard", parent ) {

    connect( this, &Keyboard::resetMappingChanged, this, [ this ] {

        if( resetMapping() ) {
            mapping().clear();
            loadDefaultMapping();
        }

    } );

}

void Keyboard::loadDefaultMapping() {

    mapping().insert( Qt::Key_A, InputDeviceEvent::A );
    mapping().insert( Qt::Key_D, InputDeviceEvent::B );
    mapping().insert( Qt::Key_W, InputDeviceEvent::Y );
    mapping().insert( Qt::Key_S, InputDeviceEvent::X );
    mapping().insert( Qt::Key_Up , InputDeviceEvent::Up );
    mapping().insert( Qt::Key_Down, InputDeviceEvent::Down );
    mapping().insert( Qt::Key_Right, InputDeviceEvent::Right );
    mapping().insert( Qt::Key_Left, InputDeviceEvent::Left );
    mapping().insert( Qt::Key_Space, InputDeviceEvent::Select );
    mapping().insert( Qt::Key_Return, InputDeviceEvent::Start );
    mapping().insert( Qt::Key_Z, InputDeviceEvent::L );
    mapping().insert( Qt::Key_X, InputDeviceEvent::R );
    mapping().insert( Qt::Key_P, InputDeviceEvent::L2 );
    mapping().insert( Qt::Key_Shift, InputDeviceEvent::R2 );
    mapping().insert( Qt::Key_N, InputDeviceEvent::L3 );
    mapping().insert( Qt::Key_M, InputDeviceEvent::R3 );

}

void Keyboard::insert( const int &event, int16_t pressed ) {

    if( editMode() ) {
        emit editModeEvent( event, pressed );
        return;
    }

    InputDeviceEvent::Event newEvent = mapping().value( event, InputDeviceEvent::Unknown );

    if( newEvent != InputDeviceEvent::Unknown ) {
        InputDevice::insert( newEvent, pressed );
    }

}

InputDeviceMapping &Keyboard::mapping() {
    return deviceMapping;
}

void Keyboard::setMapping( const QVariantMap newMapping ) {
    Q_UNUSED( newMapping );

    /*
    for ( auto &newKey : newMapping.keys() ) {
        auto newValue = newMapping.value( newKey ).toInt();

        bool foundCollision = false;
        for ( auto &oldKey : mapping().keys() ) {
            auto oldValue = InputDeviceEvent::toString( mapping().value( oldKey ) );


            if ( oldValue == newKey ) {
                foundCollision = true;
                qDebug() << name() << oldValue  << newKey  << ":" << oldKey << "==>" << QString::number( newValue );

                mapping().remove( oldKey );
                mapping().insert( QString::number( newValue ), InputDeviceEvent::toEvent( newKey ) );
                break;
            }
        }

        if ( !foundCollision ) {
            qDebug() << "no collision " << QString::number( newValue ) << mapping()[ newV ];
            mapping().insert( QString::number( newValue ), InputDeviceEvent::toEvent( newKey )  );
        }

    }
    */
    // To do...

}

bool Keyboard::loadMapping() {

    QSettings settings;

    if( !QFile::exists( settings.fileName() ) ) {
        return false;
    }

    settings.beginGroup( name() );

    for( int i = 0; i < InputDeviceEvent::Unknown; ++i ) {

        auto event = static_cast<InputDeviceEvent::Event>( i );
        auto eventString = InputDeviceEvent::toString( event );

        auto key = settings.value( eventString );

        if( key.isValid() ) {
            mapping().insert( key.toInt(), event );
        }

    }

    return !mapping().isEmpty();

}

void Keyboard::saveMapping() {

    QSettings settings;
    settings.beginGroup( name() );

    for( auto &key : mapping().keys() ) {
        auto value = mapping().value( key );
        settings.setValue( InputDeviceEvent::toString( value ), key );
    }

    qDebug() << settings.fileName();

}
