#pragma once

#include <QVector>
#include <QHash>

class Gamepad;
class SharedMemory;

class GamepadManager
{
public:
    GamepadManager();
    ~GamepadManager();

    void pollGamepads();
    void pollKeys( SharedMemory &t_memory );

    bool isEmpty() const;

public: // Iterators

    quint8 keyAt( int t_index ) {
        return m_keyboardStates[ t_index ];
    }

    const Gamepad &at( int t_index );

    int size() const;


private:
    QVector<quint8> m_keyboardStates;
    QVector<Gamepad *> m_gamepads;
    QHash<int, int> m_gamepadIndexMap;
    int m_gamepadListSize;
    bool m_initialized;

    void debugStates( Gamepad *t_gamepad );
    void debugKeyStates();
};
