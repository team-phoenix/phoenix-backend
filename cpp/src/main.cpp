#include <QCoreApplication>

#include "coredemuxer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    (void)CoreDemuxer::instance();

    return a.exec();
}
