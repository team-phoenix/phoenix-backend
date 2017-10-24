#pragma once

#include "emulator.h"

#include <QtTest>
#include <QObject>

class EmulatorIntegrationTest : public QObject {
    Q_OBJECT
    Emulator *ptr;
    QString incorrectCorePath;

private slots:

    void init() {
        ptr = new Emulator();
    }

    void cleanup() {
        delete ptr;
    }

    void should_loadCorrectCorePath() {
       // QCOMPARE( QFile::exists( workingCorePath ), true );
        //QCOMPARE( ptr->loadEmulationCore( workingCorePath ), true );
    }

    void should_notLoadIncorrectCorePath() {
        //QCOMPARE( ptr->loadEmulationCore( incorrectCorePath ), false );
    }

    void should_initializeEmulatorWithWorkingArgs() {

//        ptr->loadEmulationCore( workingCorePath );
//        ptr->loadEmulationGame( workingGamePath );

//        ptr->initEmu( workingCorePath, workingGamePath, QString( "2d" ) );

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

    }

    void should_shutdownRunningEmulatorAndNotLeaveVariablesModified() {
        ptr->shutdown();
    }

    void should_notAcceptInvalidGamePath() {
        //ptr->loadEmulationGame( incorrectGamePath );
    }


};
