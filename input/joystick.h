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
        int sdlIndex() const;
        qreal deadZone() const;
        bool analogMode() const;
        bool digitalTriggers() const;
        quint8 getButtonState( const SDL_GameControllerButton &button );

        qint16 getAxisState( const SDL_GameControllerAxis &axis );

        SDL_GameController *sdlDevice() const;
        SDL_Joystick *sdlJoystick() const;
        SDL_JoystickID instanceID() const;

        QHash< QString, int > &sdlMapping();

        void setSDLIndex( const int index );

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
        void saveMapping() override;

        void emitEditModeEvent( int event, int state );
        void emitInputDeviceEvent( InputDeviceEvent::Event event, int state );

    public slots:

        void setMapping( QVariantMap mapping ) override;


    private:

        // QML variables
        QString qmlGuid;
        QString qmlMappingString;
        int qmlInstanceID;
        int qmlSdlIndex;
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
        QHash<QString, int> sdlControllerMapping;

        void loadSDLMapping( SDL_GameController *device );

        bool hasDigitalTriggers( const QString &guid );

};

#endif // JOYSTICK_H
