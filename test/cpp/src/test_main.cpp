#include "emulatorintegrationtest.h"

//#include <stdio.h>
//#include <stdlib.h>

////#include <QObject>
#include <QDebug>
#include <QCoreApplication>
#include <QtTest>


int main(int argc, char *argv[])
{
    QCoreApplication app( argc, argv );

    app.setAttribute(Qt::AA_Use96Dpi, true);

    QTEST_SET_MAIN_SOURCE_PATH

    QList<QObject *> integrationTests = {
        new EmulatorIntegrationTest(),
    };

    int status = 0;
    for ( QObject *test : integrationTests ) {
        status |= QTest::qExec(test, argc, argv);
        delete test;
    }

    if ( status == 0 ) {
        qDebug() << "\nAll tests have successfully passed!";
    }

    qDebug() << "running test";

    return 0;

}
