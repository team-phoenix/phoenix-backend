#include "qmlinputdevice.h"

QMLInputDevice::QMLInputDevice( QObject *parent )
    : InputDevice( parent ) {
}

void QMLInputDevice::insert( const InputDeviceEvent::Event &event, const int &state ) {

    // Process the incoming event and assign it to the correct button value.
    switch( event ) {

        case InputDeviceEvent::B:
            setB( state );
            break;

        case InputDeviceEvent::A:
            setA( state );
            break;

        case InputDeviceEvent::X:
            setX( state );
            break;

        case InputDeviceEvent::Y:
            setY( state );
            break;

        // The Guide button is always recieving input, even when in game, unlike the other buttons.
        // This is so the Frontend can expose menus whenever the user hits the Guide button.
        case InputDeviceEvent::Guide:
            setGuide( state );
            break;

        case InputDeviceEvent::Start:
            setStart( state );
            break;

        case InputDeviceEvent::Select:
            setSelect( state );
            break;

        case InputDeviceEvent::Left:
            setLeft( state );
            break;

        case InputDeviceEvent::Up:
            setUp( state );
            break;

        case InputDeviceEvent::Down:
            setDown( state );
            break;

        case InputDeviceEvent::Right:
            setRight( state );
            break;

        case InputDeviceEvent::L:
            setLeftShoulder( state );
            break;

        case InputDeviceEvent::R:
            setRightShoulder( state );
            break;

        case InputDeviceEvent::L2:
            setLeftTrigger( state );
            break;

        case InputDeviceEvent::R2:
            setRightTrigger( state );
            break;

        default:
            break;

    }

}

void QMLInputDevice::setA( const bool &state ) {

    if( state == qmlA ) {
        return;
    }

    qmlA = state;
    emit aChanged();

}

void QMLInputDevice::setB( const bool &state ) {

    if( state == qmlB ) {
        return;
    }

    qmlB = state;
    emit bChanged();

}

void QMLInputDevice::setX( const bool &state ) {

    if( state == qmlX ) {
        return;
    }

    qmlX = state;
    emit xChanged();

}

void QMLInputDevice::setY( const bool &state ) {

    if( state == qmlY ) {
        return;
    }

    qmlY = state;
    emit yChanged();

}

void QMLInputDevice::setLeft( const bool &state ) {

    if( state == qmlLeft ) {
        return;
    }

    qmlLeft = state;
    emit leftChanged();

}

void QMLInputDevice::setRight( const bool &state ) {

    if( state == qmlRight ) {
        return;
    }

    qmlRight = state;
    emit rightChanged();

}

void QMLInputDevice::setUp( const bool &state ) {

    if( state == qmlUp ) {
        return;
    }

    qmlUp = state;
    emit upChanged();

}

void QMLInputDevice::setDown( const bool &state ) {

    if( state == qmlDown ) {
        return;
    }

    qmlDown = state;
    emit downChanged();

}

void QMLInputDevice::setStart( const bool &state ) {

    if( state == qmlStart ) {
        return;
    }

    qmlStart = state;
    emit startChanged();

}

void QMLInputDevice::setSelect( const bool &state ) {

    if( state == qmlSelect ) {
        return;
    }

    qmlSelect = state;
    emit selectChanged();

}

void QMLInputDevice::setGuide( const bool &state ) {

    if( state == qmlGuide ) {
        return;
    }

    qmlGuide = state;
    emit guideChanged();

}

void QMLInputDevice::setLeftShoulder( const bool &state ) {

    if( state == qmlLeftShoulder ) {
        return;
    }

    qmlLeftShoulder = state;
    emit leftShoulderChanged();

}

void QMLInputDevice::setRightShoulder( const bool &state ) {

    if( state == qmlRightShoulder ) {
        return;
    }

    qmlRightShoulder = state;
    emit rightShoulderChanged();

}

void QMLInputDevice::setLeftTrigger( const bool &state ) {

    if( state == qmlLeftTrigger ) {
        return;
    }

    qmlLeftTrigger = state;
    emit leftTriggerChanged();

}

void QMLInputDevice::setRightTrigger( const bool &state ) {

    if( state == qmlRightTrigger ) {
        return;
    }

    qmlRightTrigger = state;
    emit rightTriggerChanged();

}

bool QMLInputDevice::a() const {
    return qmlA;
}

bool QMLInputDevice::b() const {
    return qmlB;
}

bool QMLInputDevice::x() const {
    return qmlX;
}

bool QMLInputDevice::y() const {
    return qmlY;
}

bool QMLInputDevice::left() const {
    return qmlLeft;
}

bool QMLInputDevice::right() const {
    return qmlRight;
}

bool QMLInputDevice::up() const {
    return qmlUp;
}

bool QMLInputDevice::down() const {
    return qmlDown;
}
bool QMLInputDevice::start() const {
    return qmlStart;
}

bool QMLInputDevice::select() const {
    return qmlSelect;
}
bool QMLInputDevice::guide() const {
    return qmlGuide;
}

bool QMLInputDevice::leftShoulder() const {
    return qmlLeftShoulder;
}

bool QMLInputDevice::rightShoulder() const {
    return qmlRightShoulder;
}

bool QMLInputDevice::leftTrigger() const {
    return qmlLeftTrigger;
}

bool QMLInputDevice::rightTrigger() const {
    return qmlRightTrigger;
}


