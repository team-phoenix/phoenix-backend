#pragma once

#include "abstractemulator.h"

#include <QObject>
#include <QtTest>
#include <QSignalSpy>

#include <QDebug>
#include "retrocallbacks.h"

#include "emulator.h"

//class MockEmulator : public AbstractEmulator {
//public:
//    MockEmulator( QObject *parent = nullptr ) : AbstractEmulator( parent ) {

//    }

//    void run() {}
//    void init(const QString &t_corePath, const QString &t_gamePath, const QString &hwType) {}
//    void kill() {}
//    void restart() {}
//    void shutdown() {}

//    void routeAudioBatch( const qint16* data, size_t frames ) {}
//    void routeAudioSample( qint16 left, qint16 right ) {}

//    void routeVideoFrame( const char *data, quint32 width, quint32 height, size_t pitch ) {}

//};

class EmulatorUnitTests : public QObject
{
    Q_OBJECT
    AbstractEmulator *subject;

    const QString qrcCorePath = ":/snes9x_libretro.dll";
    const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

    const QString workingCorePath = QDir::temp().filePath( "tempCore.sfc" );
    const QString workingGamePath = QDir::temp().filePath( "tempGame" );

private slots:

    void initTestCase()
    {
        QFile::copy( qrcCorePath, workingCorePath );
        QFile::copy( qrcGamePath, workingGamePath );

        QCOMPARE( QFile::exists( workingCorePath ), true );
        QCOMPARE( QFile::exists( workingGamePath ), true );
    }

    void cleanupTestCase()
    {
//        QFile::remove( workingCorePath );
//        QFile::remove( workingGamePath );
    }

    void init()
    {
        subject = new Emulator( this );
        RetroCallbacks::setEmulator(subject );
    }

    void setup()
    {
        delete subject;
    }

    void should_init_with_a_working_core_and_game()
    {
        QSignalSpy spy( subject, &AbstractEmulator::initialized );

        subject->init( workingCorePath, workingGamePath, "2d" );

        QCOMPARE( spy.count(), 1 );
    }

    void should_not_init_with_bad_core()
    {
        QSignalSpy spy( subject, &AbstractEmulator::initialized );

        subject->init( "bad_core_path.bad", workingGamePath, "2d" );

        QCOMPARE( spy.count(), 0 );
    }

    void should_not_init_with_bad_game()
    {
        QSignalSpy spy( subject, &AbstractEmulator::initialized );

        subject->init( workingCorePath, "bad_game_path.bad", "2d" );

        QCOMPARE( spy.count(), 0 );
    }

};
