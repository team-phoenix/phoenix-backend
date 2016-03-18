    TEMPLATE = lib
    CONFIG += qt plugin
    QT += qml quick multimedia

    DESTDIR = plugins/PhxVideoOutput
    TARGET = phxvideooutputplugin



    INCLUDEPATH += ../../consumer ../../core ../../input ../../role ../../util ../../

HEADERS += \
    ../../consumer/videooutput.h \
    ../../backendcommon.h \
    ../../role/controllable.h \
    ../../role/consumer.h \
    ../../util/logging.h \
    ../../role/producer.h \
    ../../role/control.h \
    ../../util/controlhelper.h

SOURCES += \
    phxvideooutputplugin.cpp \
    ../../consumer/videooutput.cpp \
    ../../role/consumer.cpp \
    ../../role/controllable.cpp \
    ../../role/producer.cpp \
    ../../role/control.cpp \
    ../../util/logging.cpp

OTHER_FILES += qmldir

URI = "Phoenix/PhxVideoOutput"
unix {
    installPath = $$[QT_INSTALL_QML]/$$URI
    qmldir.path = $$installPath
    qmldir.files += $$PWD/$$OTHER_FILES
    target.path = $$installPath
    INSTALLS += target qmldir

}

# Copy the qmldir file to the same folder as the plugin binary
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$PWD/$$OTHER_FILES) $$DESTDIR), /, $$QMAKE_DIR_SEP)
