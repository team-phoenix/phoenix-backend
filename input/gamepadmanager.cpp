#include "gamepadmanager.h"
#include "logging.h"

#include <QDateTime>
#include <QQmlEngine>
#include <QEvent>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QWindow>

using namespace Input;

// The Keyboard will be always active in port 0,
// unless changed by the user.

GamepadManager::GamepadManager( QObject *parent )
    : QObject( parent ),
      m_gamepadList( 16 ),
      m_keyboardStates( 16 )
{

    for ( int i=0; i < 16; ++i ) {
        for ( int j=0; j < 16; j++ ) {
            m_gamepadStates[ i ][ j ] = 0;
        }
        m_keyboardStates[ i ] = 0;
        m_gamepadList[ i ] = nullptr;
    }

    m_keyboardMap = defaultKeyboardMap();

    connect( this, &GamepadManager::controllerDBFileChanged, &m_SDLEventLoop, &SDLEventLoop::onControllerDBFileChanged );
    connect( &m_SDLEventLoop, &SDLEventLoop::gamepadAdded, this, &GamepadManager::addGamepad );
    connect( &m_SDLEventLoop, &SDLEventLoop::gamepadRemoved, this, &GamepadManager::gamepadRemoved );

}

void GamepadManager::poll(QMutex *t_mutex, qint64 t_timeStamp ) {

    if( QDateTime::currentMSecsSinceEpoch() - t_timeStamp > 64 ) {
        //qDebug() << QDateTime::currentMSecsSinceEpoch() - t_timeStamp;
        return;
    }


    QMutexLocker locker( &interfaceMutex() );

    memset(m_gamepadStates, 0, sizeof(m_gamepadStates)); //clear buffer

    // Fetch input states
    m_SDLEventLoop.poll();

    // Copy states from gamepads and the keyboard to a buffer.
    for ( int i=0; i < 16; ++i ) {
        auto *_gPad = m_gamepadList[ i ];
        if( _gPad ) {
            for ( int b=0; b < 16; ++b ) {
                auto _gPadState = _gPad->buttonState( static_cast<Gamepad::Button>( b ) );
                m_gamepadStates[ i ][ b ] = ( 0 == i )
                        ? _gPadState | m_keyboardStates[ b ] : _gPadState;
            }

        } else {
            if ( 0 == i ) {
                for ( int b=0; b < 16; ++b ) {
                    m_gamepadStates[ i ][ b ] = m_keyboardStates[ b ];
                }
            }
        }
    }

    emit dataOut( DataReason::UpdateInput
                 , t_mutex
                 , static_cast<void *>( &m_gamepadStates )
                 , sizeof( m_gamepadStates )
                 , t_timeStamp);

    // Set final touch state once per frame
    // The flipflop is needed as the touched state may change several times *during* a frame
    // This ensures it'll get touch input for one frame if at any point during said frame it gets a touch input
    // Currently configured to keep touched state latched for 2 frames, releasing on 3rd
    // Games I've tried don't like it only going for one then off the next
    updateTouchLatch();


    // Touch input must be done before regular input as that drives frame production
//        emit producerData( QStringLiteral( "touchinput" )
//                           , &producerMutex, &touchCoords
//                           , ( size_t )touchState
//                           , currentTime );

    // Cya later buffer!
//        emit producerData( QStringLiteral( "input" )
//                           , &producerMutex
//                           , static_cast<void *>( &m_gamepadStates )
//                           , static_cast<size_t>( sizeof( m_gamepadStates ) )
//                           , timeStamp );

}

void GamepadManager::updateTouchState( QPointF t_point, bool t_pressed ) {
    if( pipeState() == PipeState::Playing ) {
        touchCoords = t_point;

        if( t_pressed ) {
            touchSet = true;
        } else {
            touchReset = true;
        }
    }
}

void GamepadManager::updateTouchLatch() {
    // Set for 2 frames, reset on 3rd
    static int setDuration = 3;
    static int frameCounter = 0;

    // qCDebug( phxInput ) << "frame start";

    // Execute transition function
    if( !touchSet && !touchReset && touchLatchState != 3 ) {
        // Latch (only if not in 3)
        touchLatchState = 0;
    }

    if( !touchSet && !touchReset && touchLatchState == 3 ) {
        // Otherwise, set if setDuration frames have not passed (should only execute once)
        // Reset if they have (should only execute once)
        if( ( frameCounter + 1 ) % setDuration == 0 ) {
            // qCDebug( phxInput ) << "    resetting" << false;
            touchLatchState = 1;
            frameCounter = 0;
        } else {
            // qCDebug( phxInput ) << "    setting" << true;
            frameCounter++;
        }
    }

    if( !touchSet && touchReset ) {
        // Reset
        // qCDebug( phxInput ) << "    normal reset";
        touchLatchState = 1;
    }

    if( touchSet && !touchReset ) {
        // Set
        // qCDebug( phxInput ) << "    normal set";
        touchLatchState = 2;
    }

    if( touchSet && touchReset ) {
        // Set (for 2 frames)
        // This is the first of the 3 frames in the sequence
        // qCDebug( phxInput ) << "    >>>event forced";
        touchLatchState = 3;
        frameCounter = 1;
    }

    // Get FF output
    switch( touchLatchState ) {
        case 0: // Latch
            break;

        case 1: // Reset
            touchState = false;
            break;

        case 2: // Set
            touchState = true;
            break;

        case 3: // Set (for a time)
            touchState = true;
            break;

        default:
            break;
    }

    // qCDebug( phxInput ) << "    frame end" << touchState;

    // Clear FF inputs
    touchSet = false;
    touchReset = false;
}

bool GamepadManager::eventFilter( QObject *object, QEvent *event ) {

    if( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease ) {
        auto *keyEvent = static_cast<QKeyEvent *>( event );
        bool pressed = ( event->type() == QEvent::KeyPress );
        auto key = static_cast<Qt::Key>( keyEvent->key() );
        if ( m_keyboardMap.contains( key ) ) {
            m_keyboardStates[ static_cast<int>( m_keyboardMap[ key ] ) ] = pressed;
            event->accept();
        }
        return true;
    }
    return QObject::eventFilter( object, event );
}

void GamepadManager::addGamepad(const Gamepad *_gamepad) {
    Q_ASSERT( _gamepad->id() < 16 );
    m_gamepadList[ _gamepad->id() ] =  _gamepad;
    emit gamepadAdded( _gamepad );
}

void GamepadManager::stateIn(PipeState t_state) {

    if( ( pipeState() == PipeState::Playing && t_state != PipeState::Playing ) ||
        ( pipeState() != PipeState::Playing && t_state == PipeState::Playing ) ) {

        if( t_state == PipeState::Playing  ) {
            qCDebug( phxInput ) << "Reading game input from keyboard";
            installKeyboardFilter();
        }

        else {
            qCDebug( phxInput ) << "No longer reading keyboard input";
            removeKeyboardFilter();
        }
    }
    setPipeState( t_state );

}

void GamepadManager::controlIn( Command t_cmd, QVariant t_data ) {
    emit controlOut( t_cmd, t_data );
}

void GamepadManager::dataIn(DataReason t_reason, QMutex *t_mutex, void *t_data, size_t t_bytes, qint64 t_timeStamp) {

    switch ( t_reason ) {
    case DataReason::PollInput:
        m_SDLEventLoop.poll();
        break;
    case DataReason::UpdateInput:
        if ( pipeState() == PipeState::Playing ) {
            poll( t_mutex, t_timeStamp );
        }
        return;

    default:
        break;
    }

    emitDataOut( t_reason, t_data, t_bytes, t_timeStamp );
}

void GamepadManager::installKeyboardFilter() {
    Q_ASSERT( QGuiApplication::topLevelWindows().size() > 0 );

    auto *window =  QGuiApplication::topLevelWindows().at( 0 );
    Q_ASSERT( window );

    window->installEventFilter( this );
}

void GamepadManager::removeKeyboardFilter() {
    Q_ASSERT( QGuiApplication::topLevelWindows().size() > 0 );

    auto *window =  QGuiApplication::topLevelWindows().at( 0 );

    Q_ASSERT( window );
    window->removeEventFilter( this );
}


