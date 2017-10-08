#include <QCoreApplication>

#if defined( RUN_UNIT_TESTS )

#include "test_emulator.h"
#include "test_libretrolibrary.h"
#include "test_messageserver.h"
#include "test_sharedmemory.h"
#include "test_gamepad.h"
#include "test_gamepadmanager.h"

#include <stdio.h>
#include <stdlib.h>

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app( argc, argv );
    app.setAttribute(Qt::AA_Use96Dpi, true);

    QTEST_SET_MAIN_SOURCE_PATH

    QList<QObject *> tests = {
        new Test_Emulator,
        new Test_LibretroLibrary,
        new Test_MessageServer,
        new Test_SharedMemory,
        new Test_Gamepad,
        new Test_GamepadManager,
    };

    int status = 0;
    for ( QObject *test : tests ) {
        status |= QTest::qExec(test, argc, argv);
        delete test;
    }

    if ( status == 0 ) {
        qDebug() << "\nAll tests have successfully passed!";
    }

    return status;
}

#else

#include "emulator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    (void)Emulator::instance();


    return a.exec();
}

#endif


