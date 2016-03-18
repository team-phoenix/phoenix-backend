
#include <qqml.h>

#include "videooutput.h"
#include "backendcommon.h"

#include "controllable.h"
#include "consumer.h"
#include "logging.h"
#include "controlhelper.h"
#include "producer.h"

#include <QQmlExtensionPlugin>

class PhxVideoOutputPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes( const char *uri ) override {
        qmlRegisterType<VideoOutput>( uri, 1, 0, "PhxVideoOutput" );

        qRegisterMetaType<Control::State>( "Control::State" );
        qRegisterMetaType<ControlHelper::State>( "ControlHelper::State" );
        qRegisterMetaType<size_t>( "size_t" );
        qRegisterMetaType<QStringMap>();
        qRegisterMetaType<ProducerFormat>( "ProducerFormat");
        qRegisterMetaType<VideoOutput *>( "VideoOutput *");
    }
};

#include "phxvideooutputplugin.moc"
