//#pragma once

//#include <QObject>
//#include <QtTest>
//#include <QDebug>
//#include <QMutex>

//#include <type_traits>
//#include <functional>

//#include "emulator.h"

//static QMutex mutex;

//static const QString workingCorePath = "/home/lee/Downloads/bsnes_balanced_libretro.so";
//static const QString incorrectCorePath = "/home/lee/Downloads/bsnes_ksksklalala.so";
//static const QString workingGamePath = "/home/lee/Downloads/bsnesdemo_v1.sfc";
//static const QString incorrectGamePath = "/home/lee/Downloads/amgmokakgakl.sfc";

//class Test_Emulator : public QObject {
//    Q_OBJECT

//    Emulator *ptr;
//    QString incorrectCorePath;

//    inline bool isValid( const char *str ) {
//        QString temp( str );
//        return !temp.isEmpty() && !temp.isNull();
//    }

//    inline bool systemInfoIsCleared() {

//        return !isValid( ptr->m_systemInfo.library_name )
//                && !isValid( ptr->m_systemInfo.library_version )
//                && !isValid( ptr->m_systemInfo.valid_extensions );
//    }

//    inline bool avInfoIsCleared() {
//        return ptr->m_avInfo.geometry.aspect_ratio == 0
//                && ptr->m_avInfo.geometry.base_height == 0
//                && ptr->m_avInfo.geometry.base_width == 0
//                && ptr->m_avInfo.geometry.max_height == 0
//                && ptr->m_avInfo.geometry.max_width == 0

//                && (float)ptr->m_avInfo.timing.fps == 0.0f
//                && (float)ptr->m_avInfo.timing.sample_rate == 0.0f;
//    }

//    inline void compare_thatMemberVariablesWereNotModified() {
//        QCOMPARE( ptr->m_libretroLibrary.isLoaded(), false );
//        QCOMPARE( ptr->m_game.isOpen(), false );
//        QCOMPARE( ptr->m_gameData.isEmpty(), true );
//        QCOMPARE( ptr->m_pixelFormat, QImage::Format_Invalid );
//        QCOMPARE( ptr->m_emuState, Emulator::State::Uninitialized );
//        QCOMPARE( ptr->m_libretroLibrary.isLoaded(), false );

//        QCOMPARE( systemInfoIsCleared(), true );
//        QCOMPARE( avInfoIsCleared(), true );
//    }

//private slots:

//    void init() {
//        mutex.lock();
//        ptr = new Emulator();
//    }

//    void cleanup() {
//        delete ptr;
//        mutex.unlock();
//    }

//    void should_loadCorrectCorePath() {
//        QCOMPARE( QFile::exists( workingCorePath ), true );
//        QCOMPARE( ptr->loadEmulationCore( workingCorePath ), true );
//    }

//    void should_notLoadIncorrectCorePath() {
//        QCOMPARE( ptr->loadEmulationCore( incorrectCorePath ), false );
//        compare_thatMemberVariablesWereNotModified();
//    }

//    void should_initializeEmulatorWithWorkingArgs() {

//        ptr->loadEmulationCore( workingCorePath );
//        ptr->loadEmulationGame( workingGamePath );

//        ptr->init( workingCorePath, workingGamePath, QString( "2d" ) );

//        // Check retro_system_info struct

//        QCOMPARE( ptr->m_libretroLibrary.isLoaded(), true );

//        // Check retro_system_info struct
//        QCOMPARE( QString( ptr->m_systemInfo.library_name ), QString( "bsnes" ) );
//        QCOMPARE( QString( ptr->m_systemInfo.library_version ), QString( "v094 (Balanced) 263e94f" ) );
//        QCOMPARE( QString( ptr->m_systemInfo.valid_extensions ), QString( "sfc|smc|bml" ) );

//        // Check retro_av_info struct

//            // Check geometry struct
//            QCOMPARE( ptr->m_avInfo.geometry.aspect_ratio, float( 4.0 / 3.0 ) );
//            QCOMPARE( ptr->m_avInfo.geometry.base_height, unsigned( 224 ) );
//            QCOMPARE( ptr->m_avInfo.geometry.base_width, unsigned( 256 ) );
//            QCOMPARE( ptr->m_avInfo.geometry.max_height, unsigned( 448 ) );
//            QCOMPARE( ptr->m_avInfo.geometry.max_width, unsigned( 512 ) );


//            // Check timing struct

//            // Some weird double comparision is going on here, truncating to float seems to solve it,
//            // and be good enough.
//            QCOMPARE( (float)ptr->m_avInfo.timing.fps, float( 60.0988 ) );
//            QCOMPARE( ptr->m_avInfo.timing.sample_rate, double( 32040.5 ) );

//        QCOMPARE( ptr->m_emuState, Emulator::State::Initialized );

//    }

//    void should_shutdownRunningEmulatorAndNotLeaveVariablesModified() {

//        should_initializeEmulatorWithWorkingArgs();

//        ptr->shutdown();

//        compare_thatMemberVariablesWereNotModified();
//    }

//    void should_notAcceptInvalidGamePath() {

//        ptr->loadEmulationGame( incorrectGamePath );

//        compare_thatMemberVariablesWereNotModified();
//    }

//};

