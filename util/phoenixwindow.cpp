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

#include <QGuiApplication>
#include <QMetaObject>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QThread>
#include <QWindow>

#include <memory>

#include "phoenixwindow.h"
#include "logging.h"

void SceneGraphHelper::setVSync( QQuickWindow *window, QOpenGLContext *context, bool vsync ) {
    context->makeCurrent( window );

#if defined( Q_OS_WIN )

    void ( *wglSwapIntervalEXT )( int ) = nullptr;

    if( ( wglSwapIntervalEXT =
              reinterpret_cast<void ( * )( int )>( context->getProcAddress( "wglSwapIntervalEXT" ) ) ) ) {
        wglSwapIntervalEXT( vsync ? 1 : 0 );
    } else {
        qWarning() << "Couldn't resolve wglSwapIntervalEXT. Unable to change VSync settings.";
    }

#elif defined( Q_OS_MACX )

    // Call a helper to execute the necessary Objective-C code
    extern void OSXSetSwapInterval( QVariant context, int interval );
    OSXSetSwapInterval( context->nativeHandle(), vsync ? 1 : 0 );

#elif defined( Q_OS_LINUX )

    void ( *glxSwapIntervalEXT )( int ) = nullptr;

    if( ( glxSwapIntervalEXT =
              reinterpret_cast<void ( * )( int )>( context->getProcAddress( "glxSwapIntervalEXT" ) ) ) ) {
        glxSwapIntervalEXT( vsync ? 1 : 0 );
    } else {
        qWarning() << "Couldn't resolve glxSwapIntervalEXT. Unable to change VSync settings.";
    }

#endif
}

PhoenixWindow::PhoenixWindow( QQuickWindow *parent ) : QQuickWindow( parent ),
    sceneGraphHelper( new SceneGraphHelper() ) {
    connect( this, &PhoenixWindow::vsyncChanged, this, &PhoenixWindow::setVsync );

    connect( this, &QQuickWindow::sceneGraphInitialized, this, [ & ]() {
        qDebug() << "Scene graph ready";
        sceneGraphIsInitialized = true;
        // qDebug() << openglContext()->format();
        sceneGraphHelper->moveToThread( openglContext()->thread() );
        emit openglContextCreated( openglContext() );
    } );

    // Grab window surface format as the OpenGL context will not be created yet
    QSurfaceFormat fmt = format();

    // Enforce the default value for vsync
    fmt.setSwapInterval( vsync ? 1 : 0 );

#if defined( Q_OS_WIN )

#elif defined( Q_OS_MACX )

    // For proper OS X fullscreen
    setFlags( flags() | Qt::WindowFullscreenButtonHint );

#endif
    //    qDebug() << fmt;
    //    qDebug() << format();
    //    qDebug() << QSurfaceFormat::defaultFormat();
    //    qDebug() << QSurfaceFormat();

    // Apply it, will be set when the context is created
    setFormat( fmt );

    //    qDebug() << format();
    update();
}

PhoenixWindow::~PhoenixWindow() {
    delete sceneGraphHelper;
}

void PhoenixWindow::setVsync( bool vsync ) {
    if( this->vsync == vsync ) {
        return;
    }

    // TODO: Cache the change so it'll get applied when it is ready
    if( !sceneGraphIsInitialized ) {
        return;
    }

    this->vsync = vsync;

    QMetaObject::invokeMethod( sceneGraphHelper, "setVSync", Qt::QueuedConnection, Q_ARG( QQuickWindow *, this ),
                               Q_ARG( QOpenGLContext *, openglContext() ), Q_ARG( bool, vsync ) );

    return;
}
