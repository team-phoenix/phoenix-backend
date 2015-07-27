#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QMap>
#include <QPair>
#include <QVector>

#include "input/inputdevice.h"
#include "libretro.h"
#include "SDL.h"
#include "SDL_gamecontroller.h"
class Joystick : public InputDevice {

    public:

        static const int maxNumOfDevices;

        explicit Joystick( const int joystickIndex, QObject *parent = 0 );
        ~Joystick();

        // Getters
        QString guid() const;
        int buttonCount() const;
        int ballCount() const;
        int hatCount() const;
        int axisCount() const;
        qreal deadZone() const;
        bool analogMode() const;
        bool digitalTriggers() const;
        quint8 getButtonState( const SDL_GameControllerButton &button );

        qint16 getAxisState( const SDL_GameControllerAxis &axis );

        SDL_GameController *sdlDevice() const;
        SDL_Joystick *sdlJoystick() const;
        SDL_JoystickID instanceID() const;

        QVariantMap &sdlMapping();

        // This value will be set to 'true' if the
        // core detects a libretro core that
        // can use the analog sticks.

        // If not, then setting this to false
        // will cause the left analog stick
        // to mimic the D-PAD.
        void setAnalogMode( const bool mode );

        // calls SDL_GameControllerClose().
        void close();

        bool loadMapping() override;
        void saveMappings() override;

        void emitEditModeEvent( int event, int state, const InputDeviceEvent::EditEventType type );
        void emitInputDeviceEvent( InputDeviceEvent::Event event, int state );

    public slots:

        bool setMappings( const QVariant key, const QVariant mapping, const InputDeviceEvent::EditEventType ) override;

    private:

        // QML variables
        QString qmlGuid;
        QString qmlMappingString;
        int qmlInstanceID;
        int qmlButtonCount;
        int qmlAxisCount;
        int qmlHatCount;
        int qmlBallCount;
        qreal qmlDeadZone;
        bool qmlAnalogMode;

        // Normal variables
        bool mDigitalTriggers;

        // Store button and axis values;
        QVector<int> mSDLButtonVector;
        QVector<int> mSDLAxisVector;

        SDL_GameController *device;

        bool loadSDLMapping( SDL_GameController *device );

        bool hasDigitalTriggers( const QString &guid );

        void fillSDLArrays();
        void fillSDLArrays( const QString &key, const int &numberValue );

};

#endif // JOYSTICK_H
