#ifndef QMLINPUTDEVICE_H
#define QMLINPUTDEVICE_H

#include "inputdevice.h"

// This QMLInputDevice is responsible for controlling the frontend, such as selecting games, and
// editing settings while using any InputDevice. The main reason for this is so the a Joystick
// instance can control the UI.

// Currently, every single InputDevice stored in the InputManager, should connect their
// InputDevice::inputDeviceEvent() signal to this classes insert() function.
// The actual button presses can then be obtained by reading the Q_PROPERTY values.

// There should only ever be one and only one QMLInputDevice every created.
class QMLInputDevice : public InputDevice {
        Q_OBJECT
        Q_PROPERTY( bool a READ a NOTIFY aChanged )
        Q_PROPERTY( bool b READ b NOTIFY bChanged )
        Q_PROPERTY( bool x READ x NOTIFY xChanged )
        Q_PROPERTY( bool y READ y NOTIFY yChanged )

        Q_PROPERTY( bool right READ right NOTIFY rightChanged )
        Q_PROPERTY( bool left READ left NOTIFY leftChanged )
        Q_PROPERTY( bool up READ up NOTIFY upChanged )
        Q_PROPERTY( bool down READ down NOTIFY downChanged )


        Q_PROPERTY( bool start READ start NOTIFY startChanged )
        Q_PROPERTY( bool select READ select NOTIFY selectChanged )
        Q_PROPERTY( bool guide READ guide NOTIFY guideChanged )

        Q_PROPERTY( bool leftShoulder READ leftShoulder NOTIFY leftShoulderChanged )
        Q_PROPERTY( bool rightShoulder READ rightShoulder NOTIFY rightShoulderChanged )
        Q_PROPERTY( bool leftTrigger READ leftTrigger NOTIFY leftTriggerChanged )
        Q_PROPERTY( bool rightTrigger READ rightTrigger NOTIFY rightTriggerChanged )

    public:

        using InputDevice::insert;

        QMLInputDevice( QObject *parent = 0 );

        bool a() const;
        bool b() const;
        bool x() const;
        bool y() const;

        bool left() const;
        bool right() const;
        bool up() const;
        bool down() const;

        bool start() const;
        bool select() const;
        bool guide() const;

        bool leftShoulder() const;
        bool rightShoulder() const;
        bool leftTrigger() const;
        bool rightTrigger() const;

    public slots:

        void insert( const InputDeviceEvent::Event &value, const int &state );

    signals:

        void aChanged();
        void bChanged();
        void xChanged();
        void yChanged();

        void leftChanged();
        void rightChanged();
        void upChanged();
        void downChanged();

        void startChanged();
        void selectChanged();
        void guideChanged();

        void leftShoulderChanged();
        void rightShoulderChanged();
        void leftTriggerChanged();
        void rightTriggerChanged();

    private:

        bool qmlA;
        bool qmlB;
        bool qmlX;
        bool qmlY;

        bool qmlLeft;
        bool qmlRight;
        bool qmlUp;
        bool qmlDown;

        bool qmlStart;
        bool qmlSelect;
        bool qmlGuide;

        bool qmlLeftShoulder;
        bool qmlRightShoulder;
        bool qmlLeftTrigger;
        bool qmlRightTrigger;

        void setA( const bool &state );
        void setB( const bool &state );
        void setX( const bool &state );
        void setY( const bool &state );

        void setLeft( const bool &state );
        void setRight( const bool &state );
        void setUp( const bool &state );
        void setDown( const bool &state );


        void setStart( const bool &state );
        void setSelect( const bool &state );
        void setGuide( const bool &state );

        void setLeftShoulder( const bool &state );
        void setRightShoulder( const bool &state );
        void setLeftTrigger( const bool &state );
        void setRightTrigger( const bool &state );

};

#endif // QMLINPUTDEVICE_H
