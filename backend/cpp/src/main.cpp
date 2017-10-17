 #include <QCoreApplication>

#include "retrocallbacks.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    RetroCallbacks::listen();

    return a.exec();
}


