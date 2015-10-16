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
        emit editModeEvent( event, pressed, InputDeviceEvent::EditEventType::ButtonEvent );
        return;
    }

    InputDeviceEvent::Event newEvent = mDeviceMapping.value( event, InputDeviceEvent::Unknown );

    if( newEvent != InputDeviceEvent::Unknown ) {
        InputDevice::insert( newEvent, pressed );
    }

}

bool Keyboard::setMappings( const QVariant key, const QVariant newMapping, const InputDeviceEvent::EditEventType type ) {
    Q_UNUSED( type );

    auto newValue = InputDeviceEvent::toEvent( key.toString() );
    auto intMapping = newMapping.toInt();


    Q_ASSERT( newValue != InputDeviceEvent::Unknown );

    bool collision = false;

    for( auto &key : mDeviceMapping.keys() ) {
        if( key == intMapping ) {
            collision = true;
            break;
        }
    }

    if( !collision ) {
        auto oldSequence = QKeySequence( mappingRef().value( key.toString() ).toString() );

        Q_ASSERT( oldSequence.count() == 1 );

        auto oldValue = oldSequence[ 0 ];

        Q_ASSERT( oldValue > 0 );
        auto removed = mDeviceMapping.remove( oldValue );
        Q_ASSERT( removed > 0 );

        mDeviceMapping.insert( intMapping, newValue );
        mappingRef().insert( key.toString(), QKeySequence( intMapping ).toString( QKeySequence::NativeText ) );
    }

    return !collision;

}

bool Keyboard::loadMapping() {

    QSettings settings;

    if( QFile::exists( settings.fileName() ) ) {

        settings.beginGroup( QStringLiteral( "Input" ) );
        auto mapping = settings.value( name() );

        auto splitValues = mapping.toString().split( "," );

        for( auto &subMapping : splitValues ) {
            auto valueList = subMapping.split( ":" );

            if( valueList.size() < 2 ) {
                continue;
            }

            auto eventString = valueList.at( 0 );
            auto mapping = valueList.at( 1 ).toInt();

            mDeviceMapping.insert( mapping, InputDeviceEvent::toEvent( eventString ) );
            mappingRef().insert( eventString, QKeySequence( mapping ).toString( QKeySequence::NativeText ) );

        }

        if( mDeviceMapping.isEmpty() ) {
            loadDefaultMapping();
        }

    } else {
        loadDefaultMapping();
    }

    return true;

}

void Keyboard::saveMappings() {

    Q_ASSERT( !mDeviceMapping.isEmpty() );

    if( mDeviceMapping.size() == 0 ) {
        return;
    }

    QSettings settings;
    settings.beginGroup( QStringLiteral( "Input" ) );

    QString mappingString = name() + QStringLiteral( "," );


    for( auto &key : mDeviceMapping.keys() ) {
        auto value = mDeviceMapping.value( key );
        mappingString += InputDeviceEvent::toString( value ) + QStringLiteral( ":" )
                         + QString::number( key ) + QStringLiteral( "," );

    }

    settings.setValue( name(), mappingString );

    qDebug() << settings.fileName();

}
