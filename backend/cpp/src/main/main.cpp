//#include "corecontroller.hpp"

//#include <QCoreApplication>
//#include <QDebug>
//#include <QThread>

//int main(int argc, char* argv[])
//{
//  QCoreApplication app(argc, argv);

//  const QString qrcCorePath = ":/snes9x_libretro.dll";
//  const QString qrcGamePath = ":/bsnesdemo_v1.sfc";

//  const QString workingCorePath = QDir::temp().filePath("tempCore.sfc");
//  const QString workingGamePath = QDir::temp().filePath("tempGame");

//  QFile::copy(qrcCorePath, workingCorePath);
//  QFile::copy(qrcGamePath, workingGamePath);

//  REQUIRE(QFile::exists(workingCorePath) == true);
//  REQUIRE(QFile::exists(workingGamePath) == true);

//  CoreController subject;

//  subject.init(workingCorePath, workingGamePath);

//  for (;;) {
//    QThread::msleep(16);
//    subject.run();
//  }

//  return app.exec();
//}


