 #include <QCoreApplication>

#include "emulator.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    (void)Emulator::instance();

    return a.exec();
}


