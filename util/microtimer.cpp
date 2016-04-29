/*
 * Copyright © 2016 athairus
 *
 * This file is part of Phoenix.
 *
 * Phoenix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "microtimer.h"

#include <QDateTime>
#include <QDebug>
#include <QEvent>

#include "logging.h"

MicroTimer::MicroTimer( Node *parent ) : Node( parent ) {
    timer.invalidate();
}

MicroTimer::~MicroTimer() {
    if( timer.isValid() || registeredTimers.size() ) {
        qWarning() << "MicroTimer: Destroyed while timer still running";
    }
}

bool MicroTimer::event( QEvent *e ) {
    // Abort if frequency hasn't been set yet, is negative or the timer is shut off
    if( frequency > 0 && timer.isValid() ) {
        // We use ms internally
        qreal currentTime = timer.nsecsElapsed() / 1000000.0;

        // Check to see if it's appropiate to send out a signal right now
        // If so, send the signal and recalculate the new target time
        if( currentTime >= targetTime ) {
            // Advance target time forward until it's at least one period past our current time
            int counter = 0;

            while( currentTime >= targetTime ) {
                targetTime += 1000.0 / frequency;
                counter++;
            }

            if( counter > 1 ) {
                //qCWarning( phxTimer ).nospace() << "Skipped " << counter - 1 << " frame(s)!";
                emit missedTimeouts( counter - 1 );
            }

            if( emitHeartbeats ) {
                commandOut( Command::Heartbeat, QVariant(), QDateTime::currentMSecsSinceEpoch() );
                emit timeout();
            }
        }
    }

    return QObject::event( e );
}

void MicroTimer::start() {
    startFreq( 0 );
}

void MicroTimer::start( qreal msec ) {
    startFreq( 1000.0 / msec );
}

void MicroTimer::startFreq( qreal frequency ) {
    if( frequency <= 0 ) {
        qWarning() << "MicroTimer: Frequency cannot be 0 or negative";
        return;
    }

    this->frequency = frequency;

    // Set inital target to the next interval
    targetTime = 1000.0 / frequency;

    // Kill all the timers
    killTimers();

    // Register these timers to generate as many events as possible to give us as many opportunities to check the freq counter as possible

    // Check time each time the window has run out of events to process
    // Causes heavy CPU usage
    if( maxAccuracy ) {
        registeredTimers << startTimer( 0, Qt::PreciseTimer );
    }

    // Check time every 1ms using native OS timers
    // Windows: Makes thread wake up every 1ms instead of the default 4ms (are you sure?)
    // OS X: Unix systems have ms accuracy
    // Linux: Unix systems have ms accuracy
    registeredTimers << startTimer( 1, Qt::PreciseTimer );

    // Begin tracking the current time
    timer.restart();
}

void MicroTimer::stop() {
    killTimers();
}

void MicroTimer::commandIn( Node::Command command, QVariant data, qint64 timeStamp ) {
    switch( command ) {
        // Eat this heartbeat if we're to emit heartbeats of our own (vsync off)
        case Command::Heartbeat: {
            if( emitHeartbeats ) {
                return;
            } else {
                Node::commandIn( command, data, timeStamp );
            }

            break;
        }

        case Command::CoreFPS: {
            Node::commandIn( command, data, timeStamp );
            qCDebug( phxTimer ).nospace() << "Began timer at " << data.toReal() << "Hz (vsync: " << !emitHeartbeats << ")";
            startFreq( data.toReal() );
            break;
        }

        // Invert vsync value to get what we should do
        case Command::SetVsync: {
            Node::commandIn( command, data, timeStamp );
            emitHeartbeats = !( data.toBool() );
            break;
        }

        default:
            Node::commandIn( command, data, timeStamp );
            break;
    }

}

void MicroTimer::killTimers() {
    timer.invalidate();

    for( int timerID : registeredTimers ) {
        killTimer( timerID );
    }

    registeredTimers.clear();
}

bool MicroTimer::isSingleShot() {
    return false;
}

void MicroTimer::setSingleShot( bool ) {
    qInfo() << "MicroTimer: Single shot timers not supported";
}

qreal MicroTimer::interval() {
    return frequency > 0 ? 1000.0 / frequency : 0;
}

void MicroTimer::setInterval( qreal interval ) {
    if( interval > 0 ) {
        frequency = interval / 1000.0;
    }
}

qreal MicroTimer::remainingTime() {
    return targetTime - timer.elapsed();
}

Qt::TimerType MicroTimer::timerType() {
    return Qt::PreciseTimer;
}

void MicroTimer::setTimerType( Qt::TimerType ) {
    qInfo() << "MicroTimer: Timer type not changeable";
}

bool MicroTimer::isActive() {
    return timer.isValid();
}

bool MicroTimer::getMaxAccuracy() {
    return maxAccuracy;
}

void MicroTimer::setMaxAccuracy( bool maxAccuracy ) {
    this->maxAccuracy = maxAccuracy;

    // Reset to apply the change unless frequency is <= 0, in which case the thing's not running and we should just
    // kill the timers if they're running
    if( frequency > 0 ) {
        startFreq( frequency );
    } else {
        killTimers();
    }
}

qreal MicroTimer::getFrequency() {
    return frequency;
}

void MicroTimer::setFrequency( qreal frequency ) {
    this->frequency = frequency;

    if( frequency <= 0 ) {
        killTimers();
    }
}
