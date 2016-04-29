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

#pragma once

#include <QObject>
#include <QQuickWindow>
#include <QSurfaceFormat>

class PhoenixWindow : public QQuickWindow {
        Q_OBJECT

    public:
        explicit PhoenixWindow( QWindow *parent = 0 );
        bool vsync{ true };

    signals:
        void vsyncChanged( bool vsync );

    public slots:
        void setVsync( bool vsync );

    private:
        void resetPlatformWindow( QSurfaceFormat fmt );
};
