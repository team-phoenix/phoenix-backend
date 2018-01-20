#include "pathcreator.h"

#include <QtGlobal>

QString PathCreator::addExtension(QString path)
{
  QString result = path;

#if defined(Q_OS_WIN)
  return result += ".dll";
#elif defined(Q_OS_LINUX)
  return result += ".so";
#elif defined(Q_OS_MACOS)
  result += ".dylib";
#else
#error OS is not supported, edit PathCreator

#endif
  return result;
}

void PathCreator::createAllAppPaths()
{
  createCorePath();
  createAssetPath();
}

QString PathCreator::corePath()
{
  return basePath() + "cores/";
}

QString PathCreator::assetPath()
{
  return basePath() + "assets/";
}

void PathCreator::createPath(QString path)
{
  QDir().mkpath(path);
}

void PathCreator::createCorePath()
{
  createPath(basePath() + "cores");
}

void PathCreator::createAssetPath()
{
  createPath(basePath() + "assets");
}

QString PathCreator::basePath()
{
  static QString path = QCoreApplication::applicationDirPath() + '/';
  return path;
}

