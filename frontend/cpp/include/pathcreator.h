#pragma once

#include <QCoreApplication>
#include <QDir>

class PathCreator
{
public:

  static QString addExtension(QString path);

  static void createAllAppPaths();

  static QString corePath();
  static QString assetPath();

private:
  static void createPath(QString path);
  static void createCorePath();
  static void createAssetPath();

  PathCreator() = default;

  static QString basePath();
};
