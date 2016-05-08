#pragma once

#include <QObject>
#include <QtPlugin>
#include <QQmlExtensionPlugin>

class BackendPlugin : public QQmlExtensionPlugin {
        Q_OBJECT
        Q_PLUGIN_METADATA( IID "org.qt-project.Qt.QQmlExtensionInterface" )
    public:
        void registerTypes( const char *uri ) override;
};