#pragma once

#include <QQmlExtensionPlugin>

// Backend
#include "consumer.h"
#include "control.h"
#include "controllable.h"
#include "controloutput.h"
#include "core.h"
#include "gameconsole.h"
#include "globalgamepad.h"
#include "inputmanager.h"
#include "libretrocore.h"
#include "phoenixwindow.h"
#include "phoenixwindownode.h"
#include "producer.h"
#include "videooutput.h"
#include "videooutputnode.h"

// Misc
#include "controlhelper.h"
#include "logging.h"


class BackendPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes( const char *uri ) override;
};
