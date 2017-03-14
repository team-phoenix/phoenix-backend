#pragma once

#include <QVector>
#include <QHash>

class Gamepad;

class GamepadManager
{
public:
    GamepadManager();
    ~GamepadManager();

    void poll();

public: // Iterators
    const Gamepad &at( int t_index );

    int size() const;

private:
    QVector<Gamepad *> m_gamepads;
    QHash<int, int> m_gamepadIndexMap;
    int m_gamepadListSize;
    bool m_initialized;

    void debugStates( Gamepad *t_gamepad );
};
