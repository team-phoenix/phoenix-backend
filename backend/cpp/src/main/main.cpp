 #include <QCoreApplication>

#include "retrocallbacks.h"
#include "emulator.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    RetroCallbacks::setEmulator( new Emulator() );

    return a.exec();
}


