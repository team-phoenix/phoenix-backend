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

// MicroTimer is an improved version of QTimer with potential microsecond accuracy and the ability to specify
// the interval as a qreal. You also gain the ability to specify the frequency (which is how the interval's internally stored)
// and to activate a max accuracy mode which further boosts the accuracy from microseconds to sub microseconds.
// Testing on Windows 10 and OS X 10.11 has shown this timer to be as accurate as +-0.1us on low system load. Your mileage may vary!
// Note that max accuracy mode causes significant CPU usage. It's recommended to place MicroTimer in its own thread,
// unless you have a thread in your program that offers a richer (larger/more frequent) set of events to work off of.
// In that case, you'd be best off using that thread and leaving max accuracy mode off. Experiment to find out what works best!

#pragma once

#include <QDebug>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <Qt>
#include <QtGlobal>

#include "controllable.h"
#include "logging.h"
#include "node.h"

class QEvent;

// FIXME: stop being Controllable
class MicroTimer : public Node, public Controllable {
        Q_OBJECT
        Q_PROPERTY( bool singleShot READ isSingleShot WRITE setSingleShot )
        Q_PROPERTY( qreal interval READ interval WRITE setInterval )
        Q_PROPERTY( qreal remainingTime READ remainingTime )
        Q_PROPERTY( Qt::TimerType timerType READ timerType WRITE setTimerType )
        Q_PROPERTY( bool active READ isActive )
        Q_PROPERTY( bool maxAccuracy READ getMaxAccuracy WRITE setMaxAccuracy )
        Q_PROPERTY( qreal frequency READ getFrequency WRITE setFrequency )

    public:
        explicit MicroTimer( Node *parent = 0 );
        virtual ~MicroTimer();
        bool event( QEvent *e ) override;

    signals:
        void timeout();
        void missedTimeouts( int numMissed );

    public slots:
        void start();
        void start( qreal msec );
        void startFreq( qreal frequency );
        void stop();

        // FIXME: Remove once we stop using Control
        void setState( Control::State currentState ) override;

        void controlIn( Command command, QVariant data, qint64 timeStamp ) override;

        // Make sure to connect to this slot to safely clean up once thread finishes
        void killTimers();

    private:
        // Property setters/getters
        bool isSingleShot();
        void setSingleShot( bool );
        qreal interval();
        void setInterval( qreal interval );
        qreal remainingTime();
        Qt::TimerType timerType();
        void setTimerType( Qt::TimerType );
        bool isActive();

        bool getMaxAccuracy();
        void setMaxAccuracy( bool maxAccuracy );
        qreal getFrequency();
        void setFrequency( qreal frequency );

        bool maxAccuracy{ false };
        QElapsedTimer timer;
        qreal frequency{ 0.0 };
        qreal targetTime{ 0 };
        QList<int> registeredTimers;
};
