//#pragma once

//#include "gamepadmanager.h"
//#include "emulator.h"
//#include "sharedmemory.h"


//#include <QObject>
//#include <QDebug>

//#include <SDL.h>

//class Test_GamepadManager : public QObject {
//    Q_OBJECT
//    GamepadManager *manager;

//private slots:

//    void initTestCase() {

//    }

//    void init() {
//        manager = new GamepadManager;
//    }

//    void cleanup() {
//        delete manager;
//    }

//    void should_initializeSDL() {
//        manager->init();
//    }

////    void should_haveKeyboardStatesFilledWithTwoAndZeroOutFromSharedMemory() {

////        SharedMemory sharedMemory;

////        const int someBufferSize = 16;
////        quint8 inputStates[ someBufferSize ];
////        for ( int i=0; i < someBufferSize; ++i ) {
////            inputStates[ i ] = 2;
////        }

////        memcpy( sharedMemory.m_memory.data(), inputStates, someBufferSize );

////        manager->m_keyboardStates.resize( someBufferSize );

////        manager->pollKeys( sharedMemory );

////        for ( int i=0; i < someBufferSize; ++i ) {
////            QCOMPARE( manager->m_keyboardStates[ i ], static_cast<quint8>( 2 ) );

////            const quint8 *data = static_cast<quint8 *>(sharedMemory.m_memory.data());
////            QCOMPARE( data[ i ], static_cast<quint8>( 0 ) );
////        }

////    }

//    void should_haveGamepadsBeOpenedDuringPolling() {

//        manager->pollGamepads();
//    }

//};
