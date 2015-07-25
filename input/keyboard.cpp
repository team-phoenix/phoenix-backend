#include "keyboard.h"

#include <QKeySequence>

Keyboard::Keyboard( QObject *parent )
    : InputDevice( LibretroType::DigitalGamepad, "Keyboard", parent ) {

    setPort( -1 );
    connect( this, &Keyboard::resetMappingChanged, this, [ this ] {

        if( resetMapping() ) {
            mDeviceMapping.clear();
            loadDefaultMapping();
        }

    } );

}

void Keyboard::loadDefaultMapping() {

    mDeviceMapping.insert( Qt::Key_A, InputDeviceEvent::A );
    mDeviceMapping.insert( Qt::Key_D, InputDeviceEvent::B );
    mDeviceMapping.insert( Qt::Key_W, InputDeviceEvent::Y );
    mDeviceMapping.insert( Qt::Key_S, InputDeviceEvent::X );
    mDeviceMapping.insert( Qt::Key_Up , InputDeviceEvent::Up );
    mDeviceMapping.insert( Qt::Key_Down, InputDeviceEvent::Down );
    mDeviceMapping.insert( Qt::Key_Right, InputDeviceEvent::Right );
    mDeviceMapping.insert( Qt::Key_Left, InputDeviceEvent::Left );
    mDeviceMapping.insert( Qt::Key_Space, InputDeviceEvent::Select );
    mDeviceMapping.insert( Qt::Key_Return, InputDeviceEvent::Start );
    mDeviceMapping.insert( Qt::Key_Z, InputDeviceEvent::L );
    mDeviceMapping.insert( Qt::Key_X, InputDeviceEvent::R );
    mDeviceMapping.insert( Qt::Key_P, InputDeviceEvent::L2 );
    mDeviceMapping.insert( Qt::Key_Shift, InputDeviceEvent::R2 );
    mDeviceMapping.insert( Qt::Key_N, InputDeviceEvent::L3 );
    mDeviceMapping.insert( Qt::Key_M, InputDeviceEvent::R3 );

    for( auto &key : mDeviceMapping.keys() ) {
        auto value = mDeviceMapping.value( key );
        mappingRef().insert( InputDeviceEvent::toString( value ), QKeySequence( key ).toString( QKeySequence::NativeText ) );
    }


}

void Keyboard::insert( const int &event, int16_t pressed ) {

    qDebug() << editMode();
    if( editMode() ) {
        emit editModeEvent( QKeySequence( event ).toString( QKeySequence::NativeText ), pressed, InputDeviceEvent::EditEventType::ButtonEvent );
        return;
    }

    InputDeviceEvent::Event newEvent = mDeviceMapping.value( event, InputDeviceEvent::Unknown );

    if( newEvent != InputDeviceEvent::Unknown ) {
        InputDevice::insert( newEvent, pressed );
    }

}

void Keyboard::setMappings( const QVariant key, const QVariant newMapping, const InputDeviceEvent::EditEventType type ) {
    Q_UNUSED( newMapping );
    Q_UNUSED( key );
    Q_UNUSED( type );

    //åmDeviceMapping.insert( mappingRef().insert(  ) );


    mappingRef().insert( key.toString(), QKeySequence( newMapping.toInt() ).toString( QKeySequence::NativeText ) );
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

    qDebug() << "LOAD MAPPING";
    loadDefaultMapping();
    return true;
    /*
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
            mDeviceMapping.insert( key.toInt(), event );
            mappingRef().insert( InputDeviceEvent::toString( event ), key );
        }

    }

    return !mDeviceMapping.isEmpty();*/

}

void Keyboard::saveMappings() {

    QSettings settings;
    settings.beginGroup( name() );

    for( auto &key : mDeviceMapping.keys() ) {
        auto value = mDeviceMapping.value( key );
        settings.setValue( InputDeviceEvent::toString( value ), key );
    }

    qDebug() << settings.fileName();

}
